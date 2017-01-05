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
void ED2K_::login() {}
void ED2K_::connect(const char *addr, unsigned short port, boost::system::error_code &err) {
    con = BuildConnector(ios, SHARED_TO(CmdH_), SHARED_TO(ConH_));
    con->mod = BuildMod();
    con->connect(addr, port, err);
}
bool ED2K_::OnConn(TCP s, const boost::system::error_code &ec) {
    printf("connected:%d\n", ec.value());
    char hash[16];
    const char *name = "testing";
    Login l(hash, name, 0, 40868);
    boost::system::error_code xec;
    s->write(l.encode(), xec);
    return true;
}
// event on socket close.
void ED2K_::OnClose(TCP s, const boost::system::error_code &ec) { printf("closed:%d\n", ec.value()); }
int ED2K_::OnCmd(Cmd c) {
    printf("data:%s\n", c->cdata());
    return 0;
}
}
}
