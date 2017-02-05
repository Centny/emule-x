//
//  runner_test.hpp
//  emule-x
//
//  Created by Centny on 2/2/17.
//
//

#ifndef runner_test_h
#define runner_test_h

#include <boost/test/included/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <fstream>
#include "../ws/curl_util.h"
#include "runner.hpp"

using namespace boost;
using namespace emulex::fs;
using namespace emulex::runner;

BOOST_AUTO_TEST_SUITE(runner)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestSending) {
    auto fdata = FData(new FData_);
    fdata->location = BuildData(".", 1);
    fdata->filename = BuildData("abc.txt", 7);
    fdata->emd4 = BuildHash(16);
    fdata->size = 513772400;
    auto file = File(new File_(fdata));
    auto ss = Sending(new Sending_);
    ss->file = file;
    ss->parts = ss->file->split(EMBLOCKSIZE);
    ss->sended = 0;
    std::vector<FilePart> ps;
    do {
        ps = ss->ed2kpart(0);
        printf("v->%lu,max(%u)\n", ss->sended, ss->parts[ss->sended - 1].second);
        BOOST_FOREACH (const FilePart& part, ps) { printf("x->%u,%u\n", part.first, part.second); }
    } while (ps.size());
    file->clear();
    printf("Sending done...\n");
}

using namespace emulex::runner;
using namespace emulex::fs;

Runner runner;
FileManager fmgr;

void runr() {
    try {
        asio::io_service ios;
        Data name = BuildData("testing", 7, true);
        Hash hash = BuildHash(16);
        hash->data[0] = 3;
        fmgr = FileManager(new FileManager_("emulex.db", "fs.db"));
        runner = Runner(new Runner_(ios, hash, name, fmgr));
        std::list<Address> srvs;
        srvs.push_back(Address(cip2long("14.23.162.173"), 4122));
        runner->start(srvs);
        auto ws = WS(new WS_(ios, 9825, "."));
        auto wraper = WsWrapper(new WsWrapper_(ws, runner));
        //
        //    auto fhash = emulex::fs::FromHex("AA4BA9F8740C596D24DC02CAE0C56CDC");
        //    auto fname = BuildData("a.7z", 5, true);
        //    auto fsize = 270053;
        //    runner->addTask(boost::filesystem::path("."), fhash, fname, fsize);
        ios.run();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(TestRunner) {
    //    if (true) {
    //        return;
    //    }
    //    auto fc=FileConf(new FileConf_(0));
    //    fc->readhash("a.7z.xdm", ALL_HASH);
    //    fc->ed2k[0]->print();
    //    std::cout << 259650 + 10403 << std::endl;
    printf("%s\n", "test runner starting...");
    boost::filesystem::remove_all(boost::filesystem::path("fs.db"));
    boost::filesystem::remove_all(boost::filesystem::path("emulex.db"));
    boost::filesystem::remove_all(boost::filesystem::path("a.7z.xdm"));
    boost::filesystem::remove_all(boost::filesystem::path("a.7z.xcm"));
    boost::thread r(&runr);
    sleep(1);
    auto res = curl_get(
        "http://127.0.0.1:9825/api/"
        "addTask?dir=.&hash=AA4BA9F8740C596D24DC02CAE0C56CDC&type=emd4&size=270053&filename=a.7z",
        0, 0);
    printf("%s\n", res->rdata);
    curl_free_res(&res);
    while (true) {
        auto taskres = curl_get("http://127.0.0.1:9825/api/listTask", 0, 0);
        auto taskj = json_tokener_parse(taskres->rdata);
//        json_object_object_get_ex(
        struct json_object *tasks_;
        json_object_object_get_ex(taskj, "tasks",&tasks_);
        auto tasks = json_object_get_array(tasks_);
        BOOST_CHECK_EQUAL(tasks->length, 1);
        auto task = (struct json_object*)tasks->array[0];
        struct json_object *status;
        json_object_object_get_ex(task, "status",&status);
        if (json_object_get_int(status) == FTSS_DONE) {
            json_object_put(taskj);
            break;
        }
        json_object_put(taskj);
        sleep(1);
    }
    printf("%s\n", "download done...");
    runner->release();
    runner->ios.stop();
    r.join();
    printf("%s\n", "test runner console done...");
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* runner_test_h */
