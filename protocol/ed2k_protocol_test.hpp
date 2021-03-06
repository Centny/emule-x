//
//  encoding_test.cpp
//  ed2k
//
//  Created by Centny on 1/2/17.
//
//
#include <boost/test/included/unit_test.hpp>
#include "ed2k_protocol.hpp"

using namespace boost;
using namespace emulex::protocol::ped2k;
using namespace boost::endian;

BOOST_AUTO_TEST_SUITE(ed2k_protocol)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TesLogin) {
    Hash hash(16);
    Data name = BuildData("abc", 3);
    Login login(hash, name, 0, 40868);
    login.show();
    //
    Login login2;
    Data xx = login.encode();
    //    login.print();
    BOOST_CHECK_EQUAL(xx->len, 60);
    login2.parse(xx);
    login2.show();
    //    BOOST_CHECK_EQUAL(strcmp(login.hash, login.hash), 0);
    BOOST_CHECK_EQUAL(login.cid, login2.cid);
    BOOST_CHECK_EQUAL(login.port, login2.port);
    BOOST_CHECK_EQUAL(strcmp(login.name->data, login.name->data), 0);
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

    login.parse(xdata);
    login.show();
    //    BOOST_CHECK_EQUAL(strcmp(login.hash, login.hash), 0);
    BOOST_CHECK_EQUAL(login.cid, 0);
    BOOST_CHECK_EQUAL(login.port, 40868);
    BOOST_CHECK_EQUAL(strcmp(login.name->data, "abc"), 0);
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
    smsg.parse(xdata);
    printf("%s->\n", smsg.c_str());
}

// BOOST_AUTO_TEST_CASE(TesSrvMe) {
//    char data[] = {0x1,  0x10, 0x2a, 0xa,  0x9d, 0xba, 0x3b, 0xe,  0x8,  0x3b, 0xc0, 0xf2, 0xb8, 0xaf, 0xdc,
//                   0xc0, 0x6f, 0xcf, 0xa,  0xd3, 0x37, 0x8,  0x36, 0x12, 0x4,  0x0,  0x0,  0x0,  0x97, 0x1,
//                   0x65, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x89, 0x11, 0x3c, 0x88, 0xfb, 0x0,  0xbd, 0x97,
//                   0x55, 0x65, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0};
//    auto xdata = BuildData(data, 59);
//    Login login;
//    login.parse(xdata);
//    login.show();
//}

BOOST_AUTO_TEST_CASE(TestSearchArgs) {
    //    try {
    SearchArgs args1("abc");
    args1.encode()->print();
    char data[] = {0x16, 0x3, 0x0, 0x61, 0x62, 0x63};
    auto xdata = BuildData(data, 6);
    SearchArgs args2;
    args2.parse(xdata);
    //    } catch (...) {
    //    }
}

// BOOST_AUTO_TEST_CASE(TestFileList) {
//    char data[] = {
//        0x33, 0x78, 0xda, 0x93, 0x65, 0x60, 0x60, 0x90, 0x7b, 0x73, 0xfe, 0x62, 0x6b, 0xdf, 0xa6, 0xde, 0xf0, 0x63,
//        0x4c, 0xdf, 0x77, 0xf9, 0x9b, 0x3d, 0xe3, 0xba, 0x6c, 0xce, 0xbb, 0x64, 0x3e, 0x0b, 0x50, 0x66, 0x1e, 0x63,
//        0x71, 0x41, 0x4e, 0x62, 0x65, 0x6a, 0x91, 0x5e, 0x51, 0xa9, 0x5e, 0x4a, 0x4e, 0x4e, 0x33, 0x13, 0x43, 0x0e,
//        0x13, 0x43, 0xa7, 0x28, 0x63, 0xa7, 0x01, 0xe3, 0x62, 0xde, 0xfa, 0x2d, 0x27, 0xcb, 0x6f, 0x5b, 0x89, 0x99,
//        0x3e, 0x79, 0x6f, 0xf7, 0x3c, 0x4a, 0x12, 0xa1, 0x6b, 0x06, 0x63, 0x6e, 0x6a, 0x4a, 0x66, 0xa2, 0x5e, 0x4a,
//        0x52, 0x07, 0x13, 0x03, 0x0f, 0x44, 0xb5, 0xe4, 0x29, 0xc6, 0xf9, 0x2b, 0x37, 0x6e, 0x2c, 0xd3, 0xfa, 0x36,
//        0x77, 0x43, 0x44, 0xe5, 0x2a, 0x71, 0x84, 0xea, 0xd9, 0x70, 0x3b, 0x52, 0x2b, 0x52, 0x9b, 0x99, 0x34, 0x3e,
//        0x1f, 0x86, 0x5a, 0x60, 0xb7, 0xed, 0xdd, 0xda, 0x45, 0x27, 0x3a, 0xa6, 0xd5, 0x7a, 0x95, 0xac, 0xdc, 0x76,
//        0x7e, 0xa2, 0x31, 0x42, 0xcb, 0x7c, 0x64, 0x2d, 0x7a, 0x45, 0xa9, 0xe9, 0x1d, 0x4c, 0x0e, 0x6f, 0x21, 0x9a,
//        0x1e, 0xbb, 0x04, 0x46, 0xdd, 0x3f, 0x52, 0xcf, 0x59, 0xc5, 0x3c, 0xe7, 0xb1, 0xf2, 0x64, 0x05, 0x5e, 0x84,
//        0xa6, 0x59, 0x40, 0x4d, 0x19, 0x89, 0xc5, 0x19, 0x50, 0x7f, 0x2c, 0x81, 0xf9, 0xa3, 0xf8, 0xf6, 0xb3, 0xca,
//        0x1f, 0xe1, 0xde, 0x69, 0xa2, 0xe5, 0x0a, 0x06, 0xa2, 0xa7, 0x6b, 0x95, 0x10, 0x3a, 0x66, 0x32, 0x96, 0xe7,
//        0x26, 0x16, 0xa7, 0x41, 0x35, 0x24, 0x31, 0x43, 0x35, 0xbc, 0x0a, 0x9b, 0x21, 0x6e, 0x7f, 0xec, 0xef, 0x3e,
//        0x83, 0x95, 0xeb, 0xdd, 0xb4, 0xe7, 0x9f, 0x70, 0x47, 0x68, 0x98, 0xce, 0x98, 0x92, 0x9a, 0x96, 0x58, 0x9a,
//        0x53, 0xd2, 0xc9, 0xc4, 0x08, 0x51, 0x7b, 0xe9, 0x54, 0xdf, 0xc2, 0xcd, 0x99, 0x42, 0x13, 0x98, 0x6e, 0x3c,
//        0xdb, 0xa7, 0x69, 0xd9, 0xce, 0x8a, 0x1c, 0xb4, 0x01, 0xb9, 0x05, 0xc1, 0x05, 0x39, 0x99, 0x25, 0x25, 0x40,
//        0x7f, 0x24, 0x56, 0x00, 0x6d, 0xd8, 0x00, 0xb3, 0x21, 0xbd, 0x3e, 0xae, 0xe7, 0xf7, 0x79, 0x2e, 0xe3, 0x2b,
//        0x0c, 0x4f, 0x8c, 0xee, 0x0a, 0x77, 0xdf, 0x40, 0x0e, 0xda, 0xc4, 0x92, 0xa2, 0x64, 0xa8, 0x8b, 0x0c, 0x18,
//        0xa1, 0xea, 0x1f, 0x1f, 0x94, 0x38, 0xbc, 0xbc, 0xfc, 0xa7, 0xc0, 0xcb, 0x82, 0x45, 0xa7, 0xba, 0x33, 0xa3,
//        0xed, 0x11, 0xea, 0x9b, 0x18, 0x05, 0x19, 0x72, 0x0b, 0x92, 0xe3, 0x73, 0x4b, 0x92, 0xf3, 0xf3, 0x4a, 0x12,
//        0x33, 0xf3, 0xa0, 0x5a, 0x19, 0x60, 0x56, 0x29, 0xc8, 0x7d, 0x0c, 0xd8, 0xb4, 0xb8, 0xfd, 0xe5, 0x39, 0x93,
//        0xd5, 0xaa, 0x6f, 0x35, 0x4b, 0x75, 0x90, 0x7d, 0x5f, 0x56, 0x60, 0x96, 0x92, 0x9a, 0x0c, 0x71, 0x1b, 0x03,
//        0x2b, 0x54, 0x83, 0x16, 0xd3, 0xa1, 0x04, 0x85, 0xc3, 0x26, 0x99, 0x73, 0xfe, 0x8b, 0x30, 0x08, 0xbd, 0x75,
//        0x9a, 0x87, 0xd0, 0x30, 0x87, 0xb1, 0x38, 0x37, 0x31, 0x39, 0xbb, 0xdc, 0xd8, 0x08, 0x6a, 0x49, 0x05, 0xcc,
//        0x7d, 0x0d, 0xf7, 0x24, 0x0b, 0x74, 0xc5, 0xb9, 0x57, 0x18, 0xf3, 0x5f, 0xfd, 0x10, 0x74, 0x4a, 0x7b, 0x21,
//        0xb6, 0x04, 0x96, 0x9e, 0x0a, 0xd5, 0xa5, 0x07, 0x8b, 0x18, 0xd9, 0x8e, 0x2f, 0x1c, 0x5f, 0x9d, 0x7a, 0x0f,
//        0x16, 0x15, 0x1e, 0xce, 0x4b, 0x0d, 0xdf, 0x78, 0x14, 0xa1, 0x6b, 0x01, 0x23, 0xd8, 0x4f, 0x15, 0xc5, 0x19,
//        0x49, 0x89, 0xc5, 0xb0, 0x84, 0xa9, 0x00, 0xd3, 0xf7, 0xe8, 0x69, 0xc3, 0x5d, 0x2e, 0x43, 0x9e, 0x4a, 0xd5,
//        0x64, 0xb7, 0xb7, 0x5b, 0x52, 0xdb, 0xcf, 0x20, 0x27, 0x81, 0xcc, 0x22, 0x13, 0xc3, 0x78, 0xa0, 0x03, 0xc1,
//        0x7e, 0xfa, 0xc4, 0x03, 0xd5, 0xf1, 0xaf, 0xdd, 0xff, 0xf3, 0xae, 0x98, 0x74, 0x89, 0x7d, 0x9f, 0x05, 0xbf,
//        0xf9, 0x69, 0xde, 0x35, 0x47, 0xf6, 0x93, 0x67, 0x98, 0x6f, 0x70, 0x7e, 0x69, 0x51, 0x72, 0x2a, 0x58, 0x8f,
//        0x46, 0x02, 0x2c, 0xe0, 0xf6, 0x3e, 0xef, 0x5a, 0xb3, 0x9a, 0xd5, 0xd0, 0xa4, 0x84, 0xc7, 0x3e, 0xa8, 0x54,
//        0x7d, 0x6e, 0x05, 0x72, 0xea, 0x84, 0xb8, 0x2e, 0xb1, 0x34, 0x25, 0x33, 0x1f, 0xea, 0xb8, 0x0b, 0x30, 0xc7,
//        0x2d, 0xe0, 0xed, 0x75, 0xb6, 0x7f, 0x2e, 0xff, 0xc0, 0x7d, 0xa3, 0x5f, 0xcc, 0x95, 0xbb, 0xbc, 0x7f, 0xb1,
//        0x25, 0xea, 0xe4, 0x8c, 0x12, 0xa8, 0xb6, 0x0e, 0x78, 0x0c, 0xdb, 0xb9, 0x55, 0x0b, 0x9e, 0xeb, 0xbf, 0x9c,
//        0x23, 0x93, 0xb3, 0xf0, 0xf6, 0x72, 0xcb, 0x78, 0xe4, 0xb0, 0x08, 0x4a, 0x4c, 0x71, 0x2f, 0x41, 0x4d, 0x49,
//        0x0d, 0x2c, 0x50, 0x7d, 0xb3, 0x2e, 0xfd, 0x54, 0xbf, 0xa7, 0xc9, 0x52, 0xbb, 0x77, 0xd7, 0xd7, 0xad, 0xcd,
//        0x2f, 0xd2, 0x4a, 0x90, 0xb3, 0x5d, 0x51, 0x4e, 0x62, 0x41, 0x2a, 0x3c, 0x82, 0xe1, 0xc1, 0x77, 0xef, 0x7e,
//        0x59, 0x28, 0xc7, 0x49, 0xff, 0xcd, 0xb7, 0xff, 0x07, 0x34, 0xfe, 0xef, 0xa8, 0xbd, 0x87, 0x9c, 0xf8, 0x52,
//        0x32, 0x8c, 0xcc, 0x4c, 0x20, 0xea, 0xe7, 0x70, 0x42, 0xd5, 0x7f, 0xbc, 0x7b, 0x8c, 0x61, 0xcb, 0xef, 0xd3,
//        0xea, 0x1a, 0x39, 0xc1, 0x86, 0xd1, 0x97, 0x1c, 0x63, 0xb1, 0x45, 0x53, 0x6e, 0x62, 0x26, 0x2c, 0x9a, 0x3e,
//        0xc0, 0xec, 0x59, 0xde, 0xbd, 0xf3, 0xa7, 0xd4, 0xae, 0x1a, 0x11, 0x0d, 0xe6, 0xca, 0x20, 0xbb, 0x70, 0xe9,
//        0x13, 0xd8, 0x12, 0x45, 0x5a, 0x11, 0x54, 0x17, 0x0f, 0x4c, 0xd7, 0xd3, 0xeb, 0x27, 0x63, 0xd8, 0x1f, 0x2b,
//        0x31, 0x84, 0x1d, 0x9e, 0x71, 0xc9, 0xe3, 0xc2, 0xe9, 0x54, 0x64, 0x0f, 0xe5, 0xe6, 0x26, 0xe6, 0x16, 0xa5,
//        0xe4, 0x56, 0x40, 0x1c, 0xf8, 0x01, 0x16, 0x53, 0x4b, 0x2b, 0xce, 0xe9, 0x30, 0x39, 0x05, 0x2b, 0xd8, 0x2f,
//        0x58, 0xe3, 0xb5, 0x42, 0xea, 0x1b, 0x1f, 0x72, 0xee, 0x50, 0x65, 0x78, 0xbe, 0x7b, 0xff, 0x8b, 0xce, 0x4d,
//        0xcf, 0x3b, 0x3b, 0x9e, 0xf6, 0x4f, 0x7b, 0xb6, 0xad, 0xe3, 0xd9, 0xb4, 0xf6, 0x27, 0xbb, 0xb7, 0x3d, 0x6d,
//        0xdd, 0xfc, 0xa2, 0x71, 0xca, 0x8b, 0x96, 0x59, 0xcf, 0xe6, 0xac, 0xd1, 0x4b, 0x4a, 0x2c, 0xe9, 0x60, 0x12,
//        0x34, 0x81, 0x18, 0x16, 0xa0, 0x25, 0x7d, 0x36, 0x4f, 0xe4, 0x47, 0x3b, 0x43, 0x97, 0x19, 0xf3, 0xaf, 0xf3,
//        0xfa, 0xcd, 0xc8, 0xf9, 0x25, 0x23, 0x31, 0x31, 0x27, 0xb3, 0x18, 0x62, 0x7b, 0x1a, 0x07, 0xd4, 0x76, 0x1d,
//        0x67, 0x1f, 0x97, 0x97, 0xcc, 0xb5, 0xcc, 0x33, 0x65, 0x65, 0x8c, 0x77, 0xed, 0x58, 0x26, 0x85, 0xec, 0xe0,
//        0xe0, 0x00, 0x88, 0x37, 0xb3, 0x53, 0x2b, 0x3b, 0x98, 0x96, 0xf9, 0x43, 0x34, 0x1c, 0x33, 0x94, 0xd8, 0x72,
//        0xa4, 0xb4, 0xf4, 0x5d, 0xbb, 0xd3, 0xb1, 0x02, 0xb1, 0xdc, 0xda, 0x64, 0x6c, 0xe1, 0x92, 0x0a, 0xcb, 0xc7,
//        0x0e, 0xb0, 0x70, 0xb9, 0xe3, 0x24, 0xbe, 0x36, 0x8f, 0xdd, 0xff, 0xd3, 0xf3, 0x0f, 0x01, 0xdb, 0x84, 0x2a,
//        0xef, 0xf8, 0x23, 0xbb, 0xab, 0x34, 0xaf, 0x28, 0x11, 0x16, 0x90, 0xd3, 0x60, 0x1a, 0x18, 0x00, 0xf2, 0x32,
//        0x1a, 0x96};
//    FileList fs;
//    auto xdata = BuildData(data, 992);
//    fs.parse(xdata);
//}

BOOST_AUTO_TEST_CASE(TestGetSource) {
    char data[] = {0x23,                                                                                           //
                   0x4b, 0x7a, 0x86, 0x0, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,  //
                   0xd7, 0x37, 0x48, 0x3};
    auto xdata = BuildData(data, 21);
    GetSource gs;
    gs.parse(xdata);
    printf("%llu\n", gs.size);
    char hash[] = {
        0x4b, 0x7a, 0x86, 0x0, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,  //
    };
    Hash hx(hash, 16);
    GetSource gs2(hx, 55064535);
    //    gs2.print();
    auto d = gs2.encode();
    d->print();
    for (int i = 0; i < 21; i++) {
        // BOOST_CHECK_EQUAL(data[i], d->data[i]);
    }
}

BOOST_AUTO_TEST_CASE(TestFoundSource) {
    char data[] = {
        0x44,                                                                                            //
        0x4b, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,  //
        0x02,                                                                                            //
        0xc0, 0xa8, 0x01, 0x70, 0xe2, 0x98, 0x01,                                                        //
        0x0a, 0xd3, 0x37, 0x0d, 0xa4, 0x9f, 0x01,                                                        //
    };
    auto xdata = BuildData(data, 32);
    FoundSource fs;
    fs.parse(xdata);
    BOOST_CHECK_EQUAL(fs.srvs.size(), 2);
    auto d = fs.encode();
    for (int i = 0; i < 32; i++) {
        BOOST_CHECK_EQUAL(data[i], d->data[i]);
    }
    //    printf("%llu\n", gs.size);
}

BOOST_AUTO_TEST_SUITE_END()
