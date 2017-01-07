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

ED2K_::ED2K_(asio::io_service &ios) : ios(ios) {}

void ED2K_::connect(const char *addr, unsigned short port, boost::system::error_code &err) {
    con = BuildConnector(ios, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    con->mod = BuildMod();
    con->connect(addr, port, err);
}

void ED2K_::OnSrvLogined() {
    //
    listServer(ecode);
//    sleep(2);
    search("a", ecode);
}

bool ED2K_::OnConn(TCP s, const boost::system::error_code &ec) {
    V_LOG_D("ED2K connected by local %s", s->address().c_str());
    printf("connected:%d\n", ec.value());
    if (s.get() == con.get()) {
        //        srv = BuildAcceptor(ios, con->local, SHARED_TO(CmdH_), SHARED_TO(ConH_));
        //        srv->reused = true;
        //        srv->mod = con->mod;
        //        srv->start(xec);
        //        printf("connected:%d,%ld\n", xec.value(),s.use_count());
        login(ecode);
    }

    return true;
}

void ED2K_::OnClose(TCP s, const boost::system::error_code &ec) { printf("closed:%d\n", ec.value()); }

int ED2K_::OnCmd(Cmd c) {
    int code = 0;
    switch (c->charAt(0)) {
        case OP_SERVERMESSAGE:
            smsg.parse(c->data);
            V_LOG_I("ED2K receive server message:%s", smsg.msg->data);
            return code;
        case OP_IDCHANGE:
            id.parse(c->data);
            V_LOG_I("ED2K change id to %ld", id.id);
            logined = true;
            OnSrvLogined();
            return code;
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
    }
    c->data->print(cbuf);
    V_LOG_W("ED2K receive unknow message:%s", cbuf);
    return 0;
}

size_t ED2K_::send(Encoding &enc, boost::system::error_code &ec) { return con->write(enc.encode(), ec); }

void ED2K_::login(boost::system::error_code &ec) {
    unsigned short port = 0;
    hash[5] = 14;
    hash[15] = 111;
    Login login(hash, name);
    send(login, ec);
    if (ec) {
        V_LOG_W("ED2K send login by name(%s),port(%d) fail with code(%d)", name, port, ec.value());
    } else {
        V_LOG_D("ED2K send login by name(%s),port(%d) success", name, port);
    }
}

void ED2K_::listServer(boost::system::error_code &ec) {
    con->write(ListServer(), ec);
    if (ec) {
        V_LOG_W("ED2K send list server fail with code(%d)", ec.value());
    } else {
        V_LOG_D("ED2K send list server %s", "success");
    }
}

void ED2K_::search(const char *key, boost::system::error_code &ec) {
    SearchArgs args(key);
//    args.print();
    send(args, ec);
    if (ec) {
        V_LOG_W("ED2K send search by key(%s) fail with code(%d)", key, ec.value());
    } else {
        V_LOG_D("ED2K send search by key(%s) success", key);
    }
}

//////////end ed2k//////////
}
}
