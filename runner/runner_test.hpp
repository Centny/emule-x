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
#include <fstream>
#include "runner.hpp"

using namespace boost;
using namespace emulex::fs;
using namespace emulex::runner;

BOOST_AUTO_TEST_SUITE(runner)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestSending) {
    auto fdata = FData(new FData_);
    fdata->filename = BuildData("abc.txt", 7);
    fdata->emd4 = BuildHash(16);
    fdata->size = 513772400;
    auto file = File(new File_(boost::filesystem::path("."), fdata));
    auto ss = Sending();
    ss.hash = fdata->emd4;
    ss.file = file;
    ss.parts = ss.file->split(EMBLOCKSIZE);
    ss.sended = 0;
    std::vector<FilePart> ps;
    do {
        ps = ss.ed2kpart();
        printf("v->%lu,max(%u)\n",ss.sended,ss.parts[ss.sended - 1].second);
        BOOST_FOREACH (const FilePart& part, ps) { printf("x->%u,%u\n", part.first, part.second); }
    } while (ps.size());
    file->clear();
    printf("Sending done...\n");
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* runner_test_h */
