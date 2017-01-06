//
//  encoding_test.cpp
//  ed2k
//
//  Created by Centny on 1/2/17.
//
//
#include <boost/test/included/unit_test.hpp>
#include "encoding.hpp"

using namespace boost;
using namespace emulex::protocol;
using namespace boost::endian;

BOOST_AUTO_TEST_SUITE(Encoding)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TesEncoding) {
    emulex::protocol::Encoding enc;
    //
    enc.reset();
    enc.put("abc", 3);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 3);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 'a');
    BOOST_CHECK_EQUAL((*enc.encode())[1], 'b');
    BOOST_CHECK_EQUAL((*enc.encode())[2], 'c');
    //
    enc.reset();
    enc.put((uint8_t)1);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 1);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 1);
    //
    enc.reset();
    enc.put((uint16_t)1);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 2);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 1);
    //
    enc.reset();
    enc.put((uint16_t)2);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 2);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 2);
    //
    enc.reset();
    enc.put((uint32_t)2);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 4);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 2);
    //
    enc.reset();
    enc.put((uint64_t)2);
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL((*enc.encode())[0], 2);
    //
    enc.reset();
    enc.putv("a", "b");
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 7);
    BOOST_CHECK_EQUAL((*enc.encode())[0], STR_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 'a');
    //
    enc.reset();
    enc.putv(0x01, "b");
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 7);
    BOOST_CHECK_EQUAL((*enc.encode())[0], STR_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 0x01);
    //
    enc.reset();
    enc.putv("a", (uint32_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL((*enc.encode())[0], INT_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 'a');
    BOOST_CHECK_EQUAL((*enc.encode())[4], 100);
    //
    enc.reset();
    enc.putv(0x01, (uint32_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 8);
    BOOST_CHECK_EQUAL((*enc.encode())[0], INT_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 0x01);
    BOOST_CHECK_EQUAL((*enc.encode())[4], 100);
    //
    enc.reset();
    enc.putv("a", (uint64_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 12);
    BOOST_CHECK_EQUAL((*enc.encode())[0], INT_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 'a');
    BOOST_CHECK_EQUAL((*enc.encode())[4], 100);
    //
    enc.reset();
    enc.putv(0x01, (uint64_t)100);
    enc.print();
    BOOST_CHECK_EQUAL(enc.size(), 12);
    BOOST_CHECK_EQUAL((*enc.encode())[0], INT_TAG);
    BOOST_CHECK_EQUAL((*enc.encode())[1], 1);
    BOOST_CHECK_EQUAL((*enc.encode())[3], 0x01);
    BOOST_CHECK_EQUAL((*enc.encode())[4], 100);
    //
    printf("%s\n", "test encoding done...");
}

BOOST_AUTO_TEST_CASE(TesLogin) {
    char hash[16];
    const char* name = "abc";
    Login login(hash, name, 0, 40868);
    login.show();
    //
    Login login2;
    Data xx = login.encode();
    login.print();
    BOOST_CHECK_EQUAL(xx->len, 60);
    BOOST_CHECK_EQUAL(login2.parse(xx), 0);
    login2.show();
    BOOST_CHECK_EQUAL(strcmp(login.hash, login.hash), 0);
    BOOST_CHECK_EQUAL(login.cid, login2.cid);
    BOOST_CHECK_EQUAL(login.port, login2.port);
    BOOST_CHECK_EQUAL(strcmp(login.name, login.name), 0);
    BOOST_CHECK_EQUAL(login.version, login2.version);
    BOOST_CHECK_EQUAL(login.flags, login2.flags);
}

BOOST_AUTO_TEST_CASE(TesLogin2) {
    char data[] = {
        0x1,                                                                                            // magic
        0x53, 0x89, 0x68, 0xe9, 0x6e, 0xe,  0xa4, 0xfa, 0xc7, 0x7d, 0x74, 0xf, 0x49, 0xb8, 0x6f, 0x6a,  // hash
        0x0,  0x0,  0x0,  0x0,                                                                          // cid
        0xa4, 0x9f,                                                                                     // port
        0x4,  0x0,  0x0,  0x0,                                                                          // count
        0x2,  0x1,  0x0,  0x1,  0x3,  0x0,  0x61, 0x62, 0x63,                                           // name
        0x3,  0x1,  0x0,  0x11, 0x3c, 0x0,  0x0,  0x0,                                                  //
        0x3,  0x1,  0x0,  0x20, 0x19, 0x3,  0x0,  0x0,                                                  //
        0x3,  0x1,  0x0,  0xfb, 0x0,  0xc8, 0x0,  0x0                                                   //
    };
    //    char data1[]={
    //        0x1 ,//
    //        0x53 ,0x89 ,0x68 ,0xe9 ,0x6e ,0xe ,0xa4 ,0xfa ,0xc7 ,0x7d ,0x74 ,0xf ,0x49 ,0xb8 ,0x6f ,0x6a ,//
    //        0x0 ,0x0 ,0x0 ,0x0 ,//
    //        0xa4 ,0x9f ,//
    //        0x4 ,0x0 ,0x0 ,0x0 ,//
    //        0x2 ,0x1 ,0x0 ,0x1 ,0x3 ,0x0 ,0x61 ,0x62 ,0x63 ,//
    //        0x3 ,0x1 ,0x0 ,0x11 ,0x3c ,0x0 ,0x0 ,0x0 ,//
    //        0x3 ,0x1 ,0x0 ,0x20 ,0x19 ,0x3 ,0x0 ,0x0 ,//
    //        0x3 ,0x1 ,0x0 ,0xfb ,0x0 ,0xc8 ,0x0 ,0x0
    //    };
    char data2[] = {0x1,                                                                                          //
                    0x58, 0xf5, 0xbf, 0x5f, 0xff, 0x7f, 0x0,  0x0,  0x82, 0x77, 0x19, 0x0,  0x1,  0x0, 0x0, 0x0,  //
                    0x0,  0x0,  0x0,  0x0,                                                                        //
                    0x36, 0x12,                                                                                   //
                    0x4,  0x0,  0x0,  0x0,                                                                        //
                    0x2,  0x1,  0x0,  0x1,  0x7,  0x0,  0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67,                 //
                    0x3,  0x1,  0x0,  0x11, 0x3c, 0x0,  0x0,  0x0,                                                //
                    0x3,  0x1,  0x0,  0x20, 0x19, 0x3,  0x0,  0x0,                                                //
                    0x3,  0x1,  0x0,  0xfb, 0xc8, 0x0,  0x0,  0x0};
    //    char data3[]={
    //        0x1 ,//
    //        0x53 ,0x89 ,0x68 ,0xe9 ,0x6e ,0xe ,0xa4 ,0xfa ,0xc7 ,0x7d ,0x74 ,0xf ,0x49 ,0xb8 ,0x6f ,0x6a ,//
    //        0x0 ,0x0 ,0x0 ,0x0 ,//
    //        0x36 ,0x12 ,//
    //        0x4 ,0x0 ,0x0 ,0x0 ,//
    //        0x2 ,0x1 ,0x0 ,0x1 ,0x7 ,0x0 ,0x74 ,0x65 ,0x73 ,0x74 ,0x69 ,0x6e ,0x67 ,0x3 ,0x1 ,0x0 ,0x11 ,0x3c ,0x0
    //        ,0x0 ,0x0 ,0x3 ,0x1 ,0x0 ,0x20 ,0x19 ,0x3 ,0x0 ,0x0 ,0x3 ,0x1 ,0x0 ,0xfb ,0xc8 ,0x0 ,0x0 ,0x0 ,0x
    //    };
    auto xdata = BuildData(data, 60);
    Login login;

    BOOST_CHECK_EQUAL(login.parse(xdata), 0);
    login.show();
    //    BOOST_CHECK_EQUAL(strcmp(login.hash, login.hash), 0);
    BOOST_CHECK_EQUAL(login.cid, 0);
    BOOST_CHECK_EQUAL(login.port, 40868);
    BOOST_CHECK_EQUAL(strcmp(login.name, "abc"), 0);
    BOOST_CHECK_EQUAL(login.version, 0x3c);
    BOOST_CHECK_EQUAL(login.flags, 0x319);
    printf("%s\n", "test login done...");
}

BOOST_AUTO_TEST_CASE(TesSrvMessage) {
    char data[] = {0x38, 0x4e, 0x0,  0x57, 0x41, 0x52, 0x4e, 0x49, 0x4e, 0x47, 0x20, 0x3a, 0x20, 0x59, 0x6f, 0x75, 0x72,
                   0x20, 0x34, 0x30, 0x38, 0x36, 0x38, 0x20, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f,
                   0x74, 0x20, 0x72, 0x65, 0x61, 0x63, 0x68, 0x61, 0x62, 0x6c, 0x65, 0x2e, 0x20, 0x50, 0x6c, 0x65, 0x61,
                   0x73, 0x65, 0x20, 0x72, 0x65, 0x76, 0x69, 0x65, 0x77, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6e, 0x65,
                   0x74, 0x77, 0x6f, 0x72, 0x6b, 0x20, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e};
    auto xdata = BuildData(data, 81);
    SrvMessage smsg;
    BOOST_CHECK_EQUAL(smsg.parse(xdata), 0);
    printf("%s->\n", smsg.c_str());
}

BOOST_AUTO_TEST_CASE(TesSrvMe) {
    char data[] = {0x1,  0x10, 0x2a, 0xa,  0x9d, 0xba, 0x3b, 0xe,  0x8,  0x3b, 0xc0, 0xf2, 0xb8, 0xaf, 0xdc,
                   0xc0, 0x6f, 0xcf, 0xa,  0xd3, 0x37, 0x8,  0x36, 0x12, 0x4,  0x0,  0x0,  0x0,  0x97, 0x1,
                   0x65, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x89, 0x11, 0x3c, 0x88, 0xfb, 0x0,  0xbd, 0x97,
                   0x55, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0};
    auto xdata = BuildData(data, 59);
    Login login;
    BOOST_CHECK_EQUAL(login.parse(xdata), 0);
    login.show();
}

BOOST_AUTO_TEST_CASE(TestSearchArgs) {
    SearchArgs args1("abc");
    args1.print();
    char data[] = {0x16, 0x1, 0x3, 0x0, 0x61, 0x62, 0x63};
    auto xdata = BuildData(data, 59);
    SearchArgs args2;
    BOOST_CHECK_EQUAL(args2.parse(xdata), 0);
}

BOOST_AUTO_TEST_SUITE_END()
