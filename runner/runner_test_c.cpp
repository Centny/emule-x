//
//  runner_console_test.cpp
//  emule-x
//
//  Created by Centny on 2/2/17.
//
//

#include <stdio.h>
#include <boost/thread/thread.hpp>
#include "runner.hpp"

using namespace emulex::runner;
using namespace emulex::fs;

Runner runner;
FileManager fmgr;

void runr() {
    asio::io_service ios;
    Data name = BuildData("testing", 7);
    Hash hash = BuildHash(16);
    hash->data[0] = 3;
    fmgr = FileManager(new FileManager_("emulex.db", "fs.db"));
    runner = Runner(new Runner_(ios, hash, name, fmgr));
    std::list<Address> srvs;
    srvs.push_back(Address(cip2long("14.23.162.173"), 4122));
    runner->start(srvs);
    //
    auto fhash = emulex::fs::FromHex("AA4BA9F8740C596D24DC02CAE0C56CDC");
    auto fname = BuildData("a.7z", 5, true);
    auto fsize = 270053;
    runner->addTask(boost::filesystem::path("."), fhash, fname, fsize);
    ios.run();
}

int runner_c(int argc, char** argv) {
    std::cout << 259650 + 10403 << std::endl;
    boost::filesystem::remove_all(boost::filesystem::path("fs.db"));
    boost::filesystem::remove_all(boost::filesystem::path("emulex.db"));
    boost::filesystem::remove_all(boost::filesystem::path("a.7z.xdm"));
    boost::filesystem::remove_all(boost::filesystem::path("a.7z.xcm"));
    boost::thread r(&runr);
    r.join();
    printf("%s\n", "test runner console done...");
}
