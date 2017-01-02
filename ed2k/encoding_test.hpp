//
//  encoding_test.cpp
//  ed2k
//
//  Created by Centny on 1/2/17.
//
//
#include "encoding.hpp"
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace emulex::ed2k;
using namespace boost::endian;

BOOST_AUTO_TEST_SUITE(Encoding) // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TesEncoding) {
  emulex::ed2k::Encoding enc;
  //
  enc.reset();
  enc.put((uint8_t)1);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 1);
  BOOST_CHECK_EQUAL(enc.data()[0], 1);
  //
  enc.reset();
  enc.put((uint16_t)1);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 2);
  BOOST_CHECK_EQUAL(enc.data()[0], 1);
  //
  enc.reset();
  enc.put((uint16_t)2);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 2);
  BOOST_CHECK_EQUAL(enc.data()[0], 2);
  //
  enc.reset();
  enc.put((uint32_t)2);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 4);
  BOOST_CHECK_EQUAL(enc.data()[0], 2);
  //
  enc.reset();
  enc.put((uint64_t)2);
  BOOST_CHECK_EQUAL(enc.size(), 8);
  BOOST_CHECK_EQUAL(enc.data()[0], 2);
  //
  enc.reset();
  enc.put("a", "b");
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 7);
  BOOST_CHECK_EQUAL(enc.data()[0], STR_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 'a');
  //
  enc.reset();
  enc.put(0x01, "b");
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 7);
  BOOST_CHECK_EQUAL(enc.data()[0], STR_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 0x01);
  //
  enc.reset();
  enc.put("a", (uint32_t)100);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 10);
  BOOST_CHECK_EQUAL(enc.data()[0], INT_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 'a');
  BOOST_CHECK_EQUAL(enc.data()[6], 100);
  //
  enc.reset();
  enc.put(0x01, (uint32_t)100);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 10);
  BOOST_CHECK_EQUAL(enc.data()[0], INT_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 0x01);
  BOOST_CHECK_EQUAL(enc.data()[6], 100);
  //
  enc.reset();
  enc.put("a", (uint64_t)100);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 14);
  BOOST_CHECK_EQUAL(enc.data()[0], INT_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 'a');
  BOOST_CHECK_EQUAL(enc.data()[6], 100);
  //
  enc.reset();
  enc.put(0x01, (uint64_t)100);
  enc.print();
  BOOST_CHECK_EQUAL(enc.size(), 14);
  BOOST_CHECK_EQUAL(enc.data()[0], INT_TAG);
  BOOST_CHECK_EQUAL(enc.data()[1], 1);
  BOOST_CHECK_EQUAL(enc.data()[3], 0x01);
  BOOST_CHECK_EQUAL(enc.data()[6], 100);
  //
  printf("%s\n", "test encoding done...");
}

BOOST_AUTO_TEST_CASE(TesLogin) {
  char hash[16];
  const char *name = "testing";
  Login login(hash, name, 0, 40868);
  login.print();
  //  emulex::ed2k::Encoding &enc = login.encode();
  //  enc.print();
  //
  Login login2;
  BOOST_CHECK_EQUAL(login2.parse(login.buf()), 0);
  login2.print();
  BOOST_CHECK_EQUAL(strcmp(login.hash, login.hash), 0);
  BOOST_CHECK_EQUAL(login.cid, login2.cid);
  BOOST_CHECK_EQUAL(login.port, login2.port);
  BOOST_CHECK_EQUAL(strcmp(login.name, login.name), 0);
  BOOST_CHECK_EQUAL(login.version, login2.version);
  BOOST_CHECK_EQUAL(login.flags, login2.flags);
  printf("%s\n", "test login done...");
}

BOOST_AUTO_TEST_SUITE_END()
