//
//  ed2k.cpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#include "ed2k.hpp"
#include "../protocol/encoding.hpp"
namespace emulex {
namespace ed2k {
using namespace emulex::protocol;

std::string con_t_cs(con_t c) {
    switch (c) {
        case ed2k_c2s:
            return std::string("ed2k_c2s");
        case ed2k_c2c:
            return std::string("ed2k_c2c");
        case ed2k_s:
            return std::string("ed2k_s");
        case defalut:
            return std::string("default");
        default:
            return std::string("unknow");
    }
}
ED2K_::ED2K_(asio::io_service &ios, Hash &hash, Data &name, Evn h) : ios(ios), hash(16) {
    mod = BuildMod();
    this->hash = hash;
    this->name = name;
    hash->data[5] = 14;
    hash->data[15] = 111;
    this->H = h;
}

Connector ED2K_::connect(con_t tag, uint32_t addr, uint16_t port, boost::system::error_code &err, uint16_t lport,
                         uint16_t pport, uint32_t from) {
    auto con = BuildConnector(ios, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    con->tag = (uint32_t)tag;
    con->mod = this->mod;
    if (lport) {
        con->setlocal(lport);
    }
    con->connect(addr, port, err);
    if (err) {
        return;
    }
    tcs[con->Id()] = con;
    esrv[con->Id()].addr = Address(addr, port);
    esrv[con->Id()].port = Port(lport, pport);
    esrv[con->Id()].from = from;
    return con->share();
}

Connector ED2K_::connect(con_t tag, const char *addr, uint16_t port, boost::system::error_code &err, uint16_t lport,
                         uint16_t pport, uint32_t from) {
    auto uaddr = boost::asio::ip::address_v4::from_string(addr).to_ulong();
    return connect(tag, uaddr, port, err, lport, pport, from);
}

// void ED2K_::OnSrvLogined() {
//
//    listServer(ecode);
//    sleep(2);
//    search("av", ecode);
//    char hash[] = {
//        0x4b, 0x7a, 0x86, 0x0, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,  //
//    };
//    size_t size = 55064535;
//    listSource(hash, size, ecode);
//}

void ED2K_::remove(TCP s) { tcs.erase(s->Id()); }

bool ED2K_::OnConn(TCP s, const boost::system::error_code &ec) {
    if (ec) {
        V_LOG_D("ED2K connect fail with code(%d)", ec.value());
        this->remove(s);
        return;
    }
    V_LOG_D("ED2K tcp(%s) connected by local %s", con_t_cs((con_t)s->tag).c_str(), s->address().c_str());
    switch (s->tag) {
        case ed2k_c2s: {
            //        srv = BuildAcceptor(ios, con->local, SHARED_TO(CmdH_), SHARED_TO(ConH_));
            //        srv->reused = true;
            //        srv->mod = con->mod;
            //        srv->start(xec);
            //        printf("connected:%d,%ld\n", xec.value(),s.use_count());
            login(s->Id(), ecode);
            break;
        }
        case ed2k_c2c: {
            hello(s->Id(), esrv[s->Id()].from, ecode);
            break;
        }
        default:
            break;
    }

    return true;
}

void ED2K_::OnClose(TCP s, const boost::system::error_code &ec) { this->remove(s); }

int ED2K_::OnCmd(Cmd c) {
    int code = 0;
    if (showlog > 1) {
        c->data->print();
    }
    switch ((uint8_t)c->charAt(0)) {
        case OP_SERVERMESSAGE: {
            SrvMessage smsg;
            smsg.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K receive server message:%s", smsg.msg->data);
            return code;
        }
        case OP_IDCHANGE: {
            auto id = esrv[c->Id()].id;
            id.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K change id to %ld", id.id);
            listServer(c->Id(), ecode);
            H->OnLogined(*this, c->Id(), id.id);
            return code;
        }
        case OP_SERVERSTATUS: {
            auto ss = esrv[c->Id()].status;
            ss.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K change server status to user(%ld),file(%ld)", ss.userc, ss.filec);
            return code;
        }
        case OP_SERVERLIST: {
            ServerList srvs;
            srvs.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse server list with %ld server found", srvs.srvs.size());
            return code;
        }
        case OP_SERVERIDENT: {
            auto sid = esrv[c->Id()].sid;
            sid.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse server identification with (%s)", sid.tostr().c_str());
            return code;
        }
        case OP_SEARCHRESULT: {
            FileList fs;
            fs.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse search result with %d found", fs.fs.size());
            H->OnFoundFile(*this, c->Id(), fs);
            return code;
        }
        case OP_FOUNDSOURCES_OBFU: {
            FoundSource found;
            found.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse found source for hash(%s) with %d server", found.hash.tostring().c_str(),
                    found.srvs.size());
            H->OnFoundSource(*this, c->Id(), found);
            return code;
        }
        case OP_CALLBACK_FAIL:
            V_LOG_I("%s", "ED2K call back request fail...");
            return code;
        case OP_REJECT:
            V_LOG_I("%s", "ED2K message rejected...");
            return code;
        case OP_CALLBACKREQUESTED: {
            CallbackRequested crs;
            crs.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse callback requested to address(%s)", addr_cs(crs).c_str());
            return code;
        }
        case OP_HELLOANSWER: {
            Hello hl(OP_HELLOANSWER);
            hl.parse(c->data, c->header->data[0]);
            esrv[c->Id()].r_lid = hl.cid;
            V_LOG_I("ED2K parse hello answer by (%s)", hl.tostr().c_str());
            H->OnAnswered(*this, c->Id());
            return code;
        }
        case OP_ACCEPTUPLOADREQ: {
            H->OnUpAccepted(*this, c->Id());
            return code;
        }
        case OP_SENDINGPART: {
            SendingPart part;
            part.parse(c->data, c->header->data[0]);
            H->OnSending(*this, c->Id(), part);
            return code;
        }
        case OP_HASHSETANSWER: {
            HashsetAnswer hs;
            hs.parse(c->data, c->header->data[0]);
            H->OnHashset(*this, c->Id(), hs);
            return code;
        }
    }
    c->data->print(cbuf);
    V_LOG_W("ED2K receive unknow message:%s", cbuf);
    return 0;
}

size_t ED2K_::write(uint64_t cid, Data data, boost::system::error_code &ec) {
    if (tcs.find(cid) == tcs.end()) {
        ec = boost::asio::error::not_connected;
        return 0;
    }
    //    V_LOG_D("%s", "ED2K_ sending data->");
    //    data->print();
    return tcs[cid]->write(data, ec);
}

size_t ED2K_::send(uint64_t cid, Encoding &enc, boost::system::error_code &ec) { return write(cid, enc.encode(), ec); }

void ED2K_::login(uint64_t cid, boost::system::error_code &ec) {
    unsigned short port = 0;
    write(cid, Login(hash, name).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send login by name(%s),port(%d) fail with code(%d)", name->data, port, ec.value());
    } else {
        V_LOG_D("ED2K send login by name(%s),port(%d) success", name->data, port);
    }
}

void ED2K_::listServer(uint64_t cid, boost::system::error_code &ec) {
    write(cid, ListServer(), ec);
    if (ec) {
        V_LOG_W("ED2K send list server fail with code(%d)", ec.value());
    } else {
        V_LOG_D("ED2K send list server %s", "success");
    }
}

void ED2K_::search(uint64_t cid, const char *key, boost::system::error_code &ec) {
    write(cid, SearchArgs(key).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send search by key(%s) fail with code(%d)", key, ec.value());
    } else {
        V_LOG_D("ED2K send search by key(%s) success", key);
    }
}

void ED2K_::listSource(uint64_t cid, Hash &hash, uint64_t size, boost::system::error_code &ec) {
    write(cid, GetSource(hash, size).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send get source by hash(%s) fail with code(%d)", hash.tostring().c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send get source by hasn(%s) success", hash.tostring().c_str());
    }
}

void ED2K_::callback(uint64_t cid, uint32_t tcid, boost::system::error_code &ec) {
    write(cid, CallbackRequest(tcid).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send callback request by cid(%lu) fail with code(%d)", cid, ec.value());
    } else {
        V_LOG_D("ED2K send callback request by cid(%lu) success", cid);
    }
}

void ED2K_::hello(uint64_t cid, uint64_t from, boost::system::error_code &ec) {
    Hello args(OP_HELLO);
    args.hash = hash;
    args.cid = esrv[from].id.id;
    args.port = esrv[from].port.second;
    args.name = name;
    args.saddr = esrv[from].addr.first;
    args.sport = esrv[from].addr.second;
    write(cid, args.encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send hello to %s fail with code(%d)", addr_cs(esrv[cid].addr).c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send hello to %s success", addr_cs(esrv[cid].addr).c_str(), cid);
    }
}

void ED2K_::uprequest(uint64_t cid, Hash &hash, boost::system::error_code &ec) {
    UploadRequest args;
    args.hash = hash;
    write(cid, args.encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send upload by hash(%s) to %s fail with code(%d)", hash.tostring().c_str(),
                addr_cs(esrv[cid].addr).c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send upload by hash(%s) to %s success", hash.tostring().c_str(), addr_cs(esrv[cid].addr).c_str(),
                cid);
    }
}
void ED2K_::request(uint64_t cid, Hash &hash, std::vector<FilePart> &parts, boost::system::error_code &ec) {
    RequestParts args;
    args.hash = hash;
    args.parts = parts;
    args.encode()->print();
    write(cid, args.encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send part request to %s fail with code(%d)", addr_cs(esrv[cid].addr).c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send part request to %s success", addr_cs(esrv[cid].addr).c_str(), cid);
    }
}
void ED2K_::request(uint64_t cid, Hash &hash, FilePart &part, boost::system::error_code &ec) {
    std::vector<FilePart> parts;
    parts.push_back(part);
    request(cid, hash, parts, ec);
}

void ED2K_::hashset(uint64_t cid, Hash &hash, boost::system::error_code &ec) {
    HashsetRequest args;
    args.hash = hash;
    write(cid, args.encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send hashset request by hash(%s) to %s fail with code(%d)", hash.tostring().c_str(),
                addr_cs(esrv[cid].addr).c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send hashset request by hash(%s) to %s success", hash.tostring().c_str(),
                addr_cs(esrv[cid].addr).c_str(), cid);
    }
}
//////////end ed2k//////////
}
}
