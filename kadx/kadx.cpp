//
//  kadx.cpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#include "kadx.hpp"

namespace emulex {
namespace kadx {

std::string ConTypeS(ConType c) {
    switch (c) {
        case C2C:
            return std::string("kadx_c2c");
        case C2S:
            return std::string("kad_c2s");
        case S2C:
            return std::string("kad_s2c");
        case Normal:
            return std::string("kad_normal");
        default:
            return std::string("unknow");
    }
}

KADX_::KADX_(asio::io_service &ios, pkadx::Login uuid, Evn h) : ios(ios) {
    mod = pkadx::BuildMod();
    this->uuid = uuid;
    this->H = h;
}

Connector KADX_::connect(ConType tag, uint32_t addr, uint16_t port, boost::system::error_code &err, uint16_t lport,
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
    //    esrv[con->Id()].addr = Address(addr, port);
    //    esrv[con->Id()].port = Port(lport, pport);
    //    esrv[con->Id()].from = from;
    return con->share();
}

Connector KADX_::connect(ConType tag, const char *addr, uint16_t port, boost::system::error_code &err, uint16_t lport,
                         uint16_t pport, uint32_t from) {
    auto uaddr = boost::asio::ip::address_v4::from_string(addr).to_ulong();
    return connect(tag, uaddr, port, err, lport, pport, from);
}

Acceptor KADX_::listen(ConType tag, const char *addr, uint16_t port, bool reused, boost::system::error_code &err) {
    auto acp = BuildAcceptor(ios, addr, port, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    acp->tag = (uint32_t)tag;
    acp->reused = reused;
    acp->start(err);
    if (err) {
        return;
    }
    tsrv[acp->Id()] = acp;
}

Monitor KADX_::monitor(ConType tag, const char *addr, uint16_t port, bool reused, boost::system::error_code &err) {
    auto m = BuildMonitor(ios, addr, port, SHARED_TO(CmdH_));
    m->tag = (uint32_t)tag;
    m->reused = reused;
    m->start(err);
    if (err) {
        return;
    }
    usrv[m->Id()] = m;
}

UDP KADX_::conto(ConType tag, const char *addr, uint16_t port, bool reused, Data data, boost::system::error_code &err) {
    auto m = BuildMonitor(ios, addr, port, SHARED_TO(CmdH_));
    m->tag = (uint32_t)tag;
    m->reused = reused;
    m->write(addr, port, data->data, data->len, err);
    // usrv[m->Id()] = m;
    ucs[m->Id()] = m;
    return m;
}

// void KADX_::OnSrvLogined() {
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

void KADX_::remove(TCP s) { tcs.erase(s->Id()); }

bool KADX_::OnConn(TCP s, const boost::system::error_code &ec) {
    if (ec) {
        V_LOG_D("KADX connect fail with code(%d)", ec.value());
        this->remove(s);
        return;
    }
    V_LOG_D("KADX tcp(%s) connected to by %s", ConTypeS((ConType)s->tag).c_str(), s->address().c_str());
    //    switch (s->tag) {
    //        case ed2k_c2s: {
    //            //        srv = BuildAcceptor(ios, con->local, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    //            //        srv->reused = true;
    //            //        srv->mod = con->mod;
    //            //        srv->start(xec);
    //            //        printf("connected:%d,%ld\n", xec.value(),s.use_count());
    //            login(s->Id(), ecode);
    //            break;
    //        }
    //        case ed2k_c2c: {
    //            hello(s->Id(), esrv[s->Id()].from, ecode);
    //            break;
    //        }
    //        default:
    //            break;
    //    }

    return true;
}

void KADX_::OnClose(TCP s, const boost::system::error_code &ec) {
    //    auto srv = esrv[s->Id()];
    //    H->OnFail(*this, srv.addr);
    this->remove(s);
}

int KADX_::OnCmd(Cmd c) {
    int code = 0;
    if (showlog > 1) {
        c->data->print();
    }
    switch ((uint8_t)c->charAt(0)) {
        case OPX_LOGIN: {
            pkadx::Login l = pkadx::Login(new pkadx::Login_);
            l->parse(c->data);
            logined[c->Id()] = l;
            c->writer->write(uuid->encode(), ecode);
            if (l->addr && l->port) {
                s2s[c->Id()] = c->writer;
                c->writer->tag = S2S;
            } else {
                s2c[c->Id()] = c->writer;
                c->writer->tag = S2C;
            }
            if (l->flags && holing.find(l->flags) != holing.end()) {
                rfilestatus(c->Id(), holing[l->flags], ecode);
            }
            break;
        }
        case OPX_LOGIN_BACK: {
            pkadx::Login l = pkadx::Login(new pkadx::Login_);
            l->parse(c->data);
            logined[c->Id()] = l;
            c2s[c->Id()] = c->writer;
            c->writer->tag = C2S;
            break;
        }
        case OPX_SEARCH: {
            pkadx::Search s = pkadx::Search(new pkadx::Search_);
            s->parse(c->data);
            pkadx::SearchBack sb = pkadx::SearchBack(new pkadx::SearchBack_(s->uid, s->tid));
            sb->fs = H->OnSearch(*this, s);
            if (sb->fs.size()) {
                auto ds = sb->encode();
                BOOST_FOREACH (const Data &d, ds) {
                    c->writer->write(d, ecode);
                    if (ecode) {
                        return;
                    }
                }
            }
            if (s2c.size()) {
                stask[s] = c->Id();
                BOOST_FOREACH (const TypeWriter &w, s2c) {
                    w.second->write(c->data, ecode);
                    if (ecode) {
                        return;
                    }
                }
            }
            //            V_LOG_W("KADX receive login message from:%s", cbuf);
            break;
        }
        case OPX_SEARCH_BACK: {
            pkadx::SearchBack sb = pkadx::SearchBack(new pkadx::SearchBack_());
            sb->parse(c->data);
            if (sb->uid->cmp(uuid->hash) == 0) {
                H->OnSearchBack(*this, sb);
                return;
            }
            if (stask.find(sb) == stask.end()) {
                return;
            }
            write(stask[sb], sb->encode()[0], ecode);
            break;
        }
        case OPX_SSRC: {
            pkadx::SearchSource ss = pkadx::SearchSource(new pkadx::SearchSource_);
            ss->parse(c->data);
            if (H->OnSearchSource(*this, ss)) {
                pkadx::FoundSource fs = pkadx::FoundSource(new pkadx::FoundSource_);
                fs->uid = ss->uid;
                fs->tid = ss->tid;
                fs->hash = ss->hash;
                fs->srvs.push_back(pkadx::KadxAddr(new pkadx::KadxAddr_(uuid->addr, uuid->port, 0)));
                c->writer->write(fs->encode(), ecode);
                if (ecode) {
                    return;
                }
            }
            if (s2c.size()) {
                stask[ss] = c->Id();
                BOOST_FOREACH (const TypeWriter &w, s2c) {
                    w.second->write(c->data, ecode);
                    if (ecode) {
                        return;
                    }
                }
            }
            break;
        }
        case OPX_FSRC: {
            pkadx::FoundSource fs = pkadx::FoundSource(new pkadx::FoundSource_());
            fs->parse(c->data);
            if (fs->uid->cmp(uuid->hash) == 0) {
                H->OnFoundSource(*this, fs);
                return;
            }
            if (stask.find(fs) == stask.end()) {
                return;
            }
            write(stask[fs], fs->encode(), ecode);
            break;
        }
        case OPX_FILE_STATUS:{
            pkadx::FileStatus fs=pkadx::FileStatus(new pkadx::FileStatus_);
            fs->parse(c->data);
            pkadx::FilePart fp=H->OnFileStatus(*this, fs);
            write(c->Id(), fs->encode(), ecode);
            break;
        }
        case OPX_FILE_STATUS_BACK:{
            pkadx::FilePart fp=pkadx::FilePart(new pkadx::FilePart_);
            fp->parse(c->data);
            H->OnFileStatusBack(*this, fp);
            break;
        }
        case OPX_FILE_PART:{
            pkadx::FilePart fp=pkadx::FilePart(new pkadx::FilePart_);
            fp->parse(c->data);
            H->OnFilePart(*this, fp);
            break;
        }
        case OPX_FILE_PROC:{
            pkadx::FileProc fp=pkadx::FileProc(new pkadx::FileProc_);
            fp->parse(c->data);
            H->OnFileProc(*this, fp);
            break;
        }
        default: {
            c->data->print(cbuf);
            V_LOG_W("ED2K receive unknow message:%s", cbuf);
            break;
        }
    }
    return 0;
}

size_t KADX_::write(uint64_t cid, Data data, boost::system::error_code &ec) {
    if (tcs.find(cid) != tcs.end()) {
        return tcs[cid]->write(data, ec);
    }
    if (ucs.find(cid) != ucs.end()) {
        return ucs[cid]->write(data->data, data->len, ec);
    }
    //    V_LOG_D("%s", "KADX_ sending data->");
    //    data->print();
    ec = boost::asio::error::not_connected;
    return 0;
}

size_t KADX_::send(uint64_t cid, Encoding &enc, boost::system::error_code &ec) { return write(cid, enc.encode(), ec); }

void KADX_::login(uint64_t cid, boost::system::error_code &ec) {
    unsigned short port = 0;
    write(cid, uuid->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send login by name(%s),port(%d) fail with code(%d)", uuid->name->data, uuid->port, ec.value());
    } else {
        V_LOG_D("KADX send login by name(%s),port(%d) success", uuid->name->data, uuid->port);
    }
}

void KADX_::search(uint64_t cid, Data &key, boost::system::error_code &ec) {
    auto s = pkadx::Search(new pkadx::Search_);
    s->uid = uuid->hash;
    s->tid = taskc++;
    s->key = key;
    write(cid, s->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send search by key(%s) fail with code(%d)", key->data, ec.value());
    } else {
        V_LOG_D("KADX send search by key(%s) success", key->data);
    }
}

void KADX_::ssrc(uint64_t cid, Hash &hash, uint64_t size, boost::system::error_code &ec) {
    auto s = pkadx::SearchSource(new pkadx::SearchSource_);
    s->uid = uuid->hash;
    s->tid = taskc++;
    s->hash = hash;
    s->size = size;
    write(cid, s->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send search source by hash(%s) fail with code(%d)", hash.tostring().c_str(), ec.value());
    } else {
        V_LOG_D("KADX send search source by hash(%s) success", hash.tostring().c_str());
    }
}

void KADX_::hole(uint64_t cid, Hash &hash, uint64_t tcid, boost::system::error_code &ec) {
    auto h = pkadx::Hole(new pkadx::Hole_);
    h->cid = tcid;
    h->flags = taskc++;
    holing[h->flags] = cid;
    write(cid, h->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send hole by cid(%llu) fail with code(%d)", tcid, ec.value());
    } else {
        V_LOG_D("KADX send hole by cid(%llu) source by hash(%s) success", tcid);
    }
}
    
void KADX_::rfilestatus(uint64_t cid, Hash &hash,  boost::system::error_code &ec){
    auto s=pkadx::FileStatus(new pkadx::FileStatus_);
    s->hash=hash;
    write(cid, s->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send rfilestatus by hash(%s) fail with code(%d)", hash.tostring().c_str(), ec.value());
    } else {
        V_LOG_D("KADX send rfilestatus by hash(%s) source by hash(%s) success", hash.tostring().c_str());
    }
}
    
void KADX_::rfilepart(uint64_t cid, Hash &hash, SortedPart& parts, boost::system::error_code &ec){
    auto fp=pkadx::FilePart(new pkadx::FilePart_);
    fp->hash=hash;
    fp->parts=parts;
    write(cid, fp->encode(), ec);
    if (ec) {
        V_LOG_W("KADX send rfilestatus by hash(%s) fail with code(%d)", hash.tostring().c_str(), ec.value());
    } else {
        V_LOG_D("KADX send rfilestatus by hash(%s) source by hash(%s) success", hash.tostring().c_str());
    }
}

void KADX_::close(uint64_t cid) {
    if (tcs.find(cid) == tcs.end()) {
        return;
    }
    auto con = tcs[cid];
    con->close();
}

void KADX_::release() {
    typedef std::map<uint64_t, TCP>::value_type tcsv;
    BOOST_FOREACH (const tcsv &t, this->tcs) { t.second->close(); }
}
}
}
