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
ED2K_::ED2K_(asio::io_service &ios) : ios(ios) { mod = BuildMod(); }

Connector ED2K_::connect(con_t tag, const char *addr, unsigned short port, boost::system::error_code &err) {
    auto con = BuildConnector(ios, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    con->tag = (uint32_t)tag;
    con->mod = this->mod;
    con->connect(addr, port, err);
    if (err) {
        return;
    }
    tcs[con->Id()] = con;
    return con->share();
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
    if (s->tag == ed2k_c2s) {
        //        srv = BuildAcceptor(ios, con->local, SHARED_TO(CmdH_), SHARED_TO(ConH_));
        //        srv->reused = true;
        //        srv->mod = con->mod;
        //        srv->start(xec);
        //        printf("connected:%d,%ld\n", xec.value(),s.use_count());
        login(s->Id(), ecode);
    }

    return true;
}

void ED2K_::OnClose(TCP s, const boost::system::error_code &ec) { this->remove(s); }

int ED2K_::OnCmd(Cmd c) {
    int code = 0;
    switch ((uint8_t)c->charAt(0)) {
        case OP_SERVERMESSAGE: {
            SrvMessage smsg;
            smsg.parse(c->data);
            V_LOG_I("ED2K receive server message:%s", smsg.msg->data);
            return code;
        }
        case OP_IDCHANGE: {
            IDCHANGE id;
            id.parse(c->data);
            V_LOG_I("ED2K change id to %ld", id.id);
            ids[c->Id()] = id;
            listServer(c->Id(), ecode);
            return code;
        }
        case OP_SERVERSTATUS:
            status.parse(c->data);
            V_LOG_I("ED2K change server status to user(%ld),file(%ld)", status.userc, status.filec);
            return code;
        case OP_SERVERLIST:
            srvs.parse(c->data);
            V_LOG_I("ED2K parse server list with %ld server found", srvs.srvs.size());
            return code;
        case OP_SERVERIDENT:
            sid.parse(c->data);
            V_LOG_I("ED2K parse server identification with server name(%s,%s)", sid.name, sid.desc);
            return code;
        case OP_SEARCHRESULT:
            fs.parse(c->data, c->header->data[0]);
            V_LOG_I("ED2K parse search result with %d found", fs.fs.size());
            return code;
        case OP_FOUNDSOURCES_OBFU:
            found.parse(c->data);
            V_LOG_I("ED2K parse found source for hash(%s) with %d server", found.hash.tostring().c_str(),
                    found.srvs.size());
            return code;
        case OP_CALLBACK_FAIL:
            V_LOG_I("%s", "ED2K call back request fail...");
            return code;
        case OP_REJECT:
            V_LOG_I("%s", "ED2K message rejected...");
            return code;
        case OP_CALLBACKREQUESTED:
            CallbackRequested crs;
            crs.parse(c->data);
            V_LOG_I("ED2K parse callback requested to address(%s)", addr_cs(crs).c_str());
            return code;
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
    return tcs[cid]->write(data, ec);
}

size_t ED2K_::send(uint64_t cid, Encoding &enc, boost::system::error_code &ec) { return write(cid, enc.encode(), ec); }

void ED2K_::login(uint64_t cid, boost::system::error_code &ec) {
    unsigned short port = 0;
    hash[5] = 14;
    hash[15] = 111;
    write(cid, Login(hash, name).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send login by name(%s),port(%d) fail with code(%d)", name, port, ec.value());
    } else {
        V_LOG_D("ED2K send login by name(%s),port(%d) success", name, port);
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

void ED2K_::listSource(uint64_t cid, const char *hash, uint64_t size, boost::system::error_code &ec) {
    write(cid, GetSource(hash, size).encode(), ec);
    if (ec) {
        V_LOG_W("ED2K send get source by hash(%s) fail with code(%d)", hash_tos(hash).c_str(), ec.value());
    } else {
        V_LOG_D("ED2K send get source by hasn(%s) success", hash_tos(hash).c_str());
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

//////////end ed2k//////////
}
}
