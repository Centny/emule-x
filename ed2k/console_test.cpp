//
//  ed2k_console_test.cpp
//  emule-x
//
//  Created by Centny on 1/5/17.
//
//

#include <stdio.h>
#include "ed2k.hpp"

using namespace boost;
using namespace emulex::ed2k;

std::string valxx() { return std::string("abc"); }

int edk_console(int argc, char **argv) {
    printf("%0x\n", 0 << 17 | 50 << 10 | 0 << 7);
    auto xxd = valxx();
    asio::io_service ios;
    ED2K ed2k(new ED2K_(ios));
    boost::system::error_code err;
    //    ed2k->connect("10.211.55.8", 4122, err);
    ed2k->connect("127.0.0.1", 20000, err);
    printf("ec:%d \n", err.value());
    ios.run();
    printf("%s\n", "test ed2k console done...");
}
