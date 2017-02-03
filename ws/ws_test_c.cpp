//
//  ws_test_c.cpp
//  emule-x
//
//  Created by Centny on 2/3/17.
//
//

#include <stdio.h>
#include <boost/thread/thread.hpp>
#include "ws.hpp"

using namespace emulex::ws;

int run_ws_c(int argc, char** argv) {
    boost::asio::io_service ios;
    WS ws = WS(new WS_(ios, 8234, "./"));
    ios.run();
    printf("%s\n", "test run_ws console done...");
}
