//
//  encoding_test.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef encoding_test_h
#define encoding_test_h
#include "encoding.hpp"
BOOST_AUTO_TEST_SUITE(Encoding)
BOOST_AUTO_TEST_CASE(TesEncoding) {
    emulex::encoding::Encoding enc;
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

BOOST_AUTO_TEST_SUITE_END()

#endif /* encoding_test_h */
