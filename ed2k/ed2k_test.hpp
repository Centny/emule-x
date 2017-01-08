#include "ed2k.hpp"

#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace emulex::ed2k;

BOOST_AUTO_TEST_SUITE(Ed2k)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TesEd2k) {
    return;
    asio::io_service ios;
    ED2K ed2k(new ED2K_(ios));
    boost::system::error_code err;
    ed2k->connect(ed2k_c2s, "10.211.55.8", 4122, err);
    printf("ec:%d \n", err.value());
    BOOST_CHECK(err == 0);
    ios.run();
    printf("%s\n", "test ed2k done...");
}

BOOST_AUTO_TEST_SUITE_END()
