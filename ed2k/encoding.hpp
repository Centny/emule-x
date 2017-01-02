//
//  encoding.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef encoding_hpp
#define encoding_hpp
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

namespace emulex {
namespace ed2k {
using namespace boost;
const uint8_t PROTOCOL = 0xE3;
const uint8_t OP_LOGINREQUEST = 0x01;
const uint8_t STR_TAG = 0x2;
const uint8_t INT_TAG = 0x3;
const uint8_t FLOAT_TAG = 0x4;

#define LittleEndian endian::endian_buffer<endian::order::little, T, n_bits>

/*
 the ed2k base encoding for int/string/tag
 */
class Encoding {
public:
  Encoding();
  void reset();
  const char *data();
  size_t size();
  boost::asio::streambuf &buf();
  template <typename T, std::size_t n_bits> // the put template
  Encoding &put(T val);
  Encoding &put(const char *val, size_t len);
  Encoding &put(uint8_t val);
  Encoding &put(uint16_t val);
  Encoding &put(uint32_t val);
  Encoding &put(uint64_t val);
  //    Encoding& put(float val);
  Encoding &put(const char *name, const char *val);
  Encoding &put(uint8_t name, const char *val);
  Encoding &put(const char *name, uint32_t val);
  Encoding &put(uint8_t name, uint32_t val);
  //  Encoding &put(const char *name, float val);
  //  Encoding &put(uint8_t name, float val);
  Encoding &put(const char *name, uint64_t val);
  Encoding &put(uint8_t name, uint64_t val);
  void print(char *buf = 0);

protected:
  boost::asio::streambuf buf_;
};

  /*
   the ed2k login frame.
   */
class Decoding {
public:
  Decoding(boost::asio::streambuf &buf);
  template <typename T, std::size_t n_bits> // the get template
  int get(T &t);
  int get(char *name, size_t len);
  int get(char *name, uint16_t &nlen);
  int get(char *name, uint16_t &nlen, char *value, uint16_t &vlen);

protected:
  boost::asio::streambuf &buf_;
};

  /*
   the ed2k base decoding for int/string/tag
   */
class Login {
public:
  char hash[16];
  uint32_t cid;
  uint16_t port;
  char name[256];
  uint64_t version;
  uint64_t flags;

protected:
  Encoding enc;

public:
  Login();
  Login(const char *hash, const char *name, uint32_t cid = 0,
        uint16_t port = 4662, uint64_t version = 0x3C, uint64_t flags = 0x01);
  Encoding &encode();
  boost::asio::streambuf &buf();
  int parse(boost::asio::streambuf &buf);
  void print(char *buf = 0);
};
  //
}
}

#endif /* encoding_hpp */
