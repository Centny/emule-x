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

KADX_::KADX_(asio::io_service &ios, Hash &hash, Data &name, Evn h) : ios(ios), hash(16) {
    //    mod = BuildMod();
    this->hash = hash;
    this->name = name;
    hash->data[5] = 14;
    hash->data[15] = 111;
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
    usrv[m->Id()] = m;
    auto con = m->con();
    ucs[con->Id()] = con;
    return con;
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
    switch ((uint8_t)c->charAt(0)) {}
    c->data->print(cbuf);
    V_LOG_W("ED2K receive unknow message:%s", cbuf);
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
