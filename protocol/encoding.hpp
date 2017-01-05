//
//  encoding.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef encoding_hpp
#define encoding_hpp
#include <stdio.h>
#include <boost-utils/boost-utils.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace emulex {
namespace protocol {
using namespace boost;
using namespace butils::netw;
const uint8_t PROTOCOL = 0xE3;
const uint8_t OP_LOGINREQUEST = 0x01;
const uint8_t STR_TAG = 0x2;
const uint8_t INT_TAG = 0x3;
const uint8_t FLOAT_TAG = 0x4;

ModH BuildMod();
/*
 the ed2k base encoding for int/string/tag
 */
class Encoding {
   public:
    Encoding();
    void reset();
    size_t size();
    boost::asio::streambuf &buf();
    Data encode();
    Encoding &put(const char *val, size_t len);
    Encoding &put(uint8_t val);
    Encoding &put(uint16_t val);
    Encoding &put(uint32_t val);
    Encoding &put(uint64_t val);
    //    Encoding& put(float val);
    Encoding &putv(const char *name, const char *val);
    Encoding &putv(uint8_t name, const char *val);
    Encoding &putv(const char *name, uint32_t val);
    Encoding &putv(uint8_t name, uint32_t val);
    //  Encoding &put(const char *name, float val);
    //  Encoding &put(uint8_t name, float val);
    Encoding &putv(const char *name, uint64_t val);
    Encoding &putv(uint8_t name, uint64_t val);
    void print(char *buf = 0);

   protected:
    boost::asio::streambuf buf_;
};

/*
 the ed2k login frame.
 */
class Decoding {
   public:
    Decoding(Data &data);
    virtual ~Decoding();
    template <typename T, std::size_t n_bits>
    int get(T &val) {
        if (data->len - offset < n_bits) {
            return -1;
        }
        val = boost::endian::detail::load_little_endian<T, n_bits>(data->data + offset);
        offset += n_bits;
        return 0;
    }
    int get(char *name, size_t len);
    int get(char *name, uint16_t &nlen);
    int get(char *name, uint16_t &nlen, char *value, uint16_t &vlen);
    int get(char *name, uint16_t &nlen, uint32_t &vlen);

   public:
    Data data;
    size_t offset;
};

/*
 the ed2k base decoding for int/string/tag
 */
class Login {
   public:
    char hash[16];
    uint32_t cid;
    uint16_t port, port2;
    char name[256];
    uint32_t version;
    uint32_t flags;

   protected:
    Encoding enc;

   public:
    Login();
    Login(const char *hash, const char *name, uint32_t cid = 0, uint16_t port = 4662, uint64_t version = 0x3C,
          uint64_t flags = 0x01);
    Encoding &encoder();
    Data encode();
    int parse(Data &data);
    void print(char *buf = 0);
};
//
}
}

#endif /* encoding_hpp */
