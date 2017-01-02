//
//  encoding.cpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#include "encoding.hpp"

namespace emulex {
namespace ed2k {
using namespace boost::endian::detail;
Encoding::Encoding() {}

void Encoding::reset() { buf_.consume(buf_.size()); }

const char *Encoding::Encoding::data() {
  return asio::buffer_cast<const char *>(buf_.data());
}

size_t Encoding::Encoding::size() { return buf_.size(); }

asio::streambuf &Encoding::Encoding::buf() { return buf_; }

template <typename T, std::size_t n_bits> // the put template
Encoding &Encoding::put(T val) {
  const char *data = LittleEndian(val).data();
  buf_.sputn(data, n_bits / 8);
  return *this;
}

Encoding &Encoding::put(const char *val, size_t len) {
  buf_.sputn(val, len);
  return *this;
}
Encoding &Encoding::put(uint8_t val) {
  put<uint8_t, 8>(val);
  return *this;
}
Encoding &Encoding::put(uint16_t val) {
  put<uint16_t, 16>(val);
  return *this;
}
Encoding &Encoding::put(uint32_t val) {
  put<uint32_t, 32>(val);
  return *this;
}
Encoding &Encoding::put(uint64_t val) {
  put<uint64_t, 64>(val);
  return *this;
}
//    Encoding& Encoding::put(float val){
//      LittleEndianBuf::put<uint32_t, 32>(val);
//      return *this;
//    }
Encoding &Encoding::put(const char *name, const char *val) {
  put(STR_TAG);
  put((uint16_t)strlen(name));
  put(name, strlen(name));
  put((uint16_t)strlen(val));
  put(val, strlen(val));
}
Encoding &Encoding::put(uint8_t name, const char *val) {
  put(STR_TAG);
  put((uint16_t)1);
  put(name);
  put((uint16_t)strlen(val));
  put(val, strlen(val));
}
Encoding &Encoding::put(const char *name, uint32_t val) {
  put(INT_TAG);
  put((uint16_t)strlen(name));
  put(name, strlen(name));
  put((uint16_t)4);
  put(val);
}
Encoding &Encoding::put(uint8_t name, uint32_t val) {
  put(INT_TAG);
  put((uint16_t)1);
  put(name);
  put((uint16_t)4);
  put(val);
}
//    Encoding& Encoding::put(const char* name, float val){
//      put(FLOAT_TAG);
//      put((uint16_t)strlen(name));
//      put(name, strlen(name));
//      put((uint16_t)4);
//      put(val);
//    }
//    Encoding& Encoding::put(uint8_t name, float val){
//      put(FLOAT_TAG);
//      put((uint16_t)1);
//      put(name);
//      put((uint16_t)4);
//      put(val);
//    }

Encoding &Encoding::put(const char *name, uint64_t val) {
  put(INT_TAG);
  put((uint16_t)strlen(name));
  put(name, strlen(name));
  put((uint16_t)8);
  put(val);
}
Encoding &Encoding::put(uint8_t name, uint64_t val) {
  put(INT_TAG);
  put((uint16_t)1);
  put(name);
  put((uint16_t)8);
  put(val);
}

void Encoding::print(char *buf) {
  size_t len = size();
  const char *tdata = data();
  char tbuf_[102400];
  char *tbuf = buf;
  if (tbuf == 0) {
    tbuf = tbuf_;
  }
  size_t tlen = sprintf(tbuf, "ENC(%ld):", len);
  for (size_t i = 0; i < len; i++) {
    tlen += sprintf(tbuf + tlen, "%2x ", (uint8_t)tdata[i]);
  }
  tbuf[tlen] = 0;
  if (buf == 0) {
    printf("%s\n", tbuf);
  }
}

Decoding::Decoding(boost::asio::streambuf &buf) : buf_(buf) {}

template <typename T, std::size_t n_bits> // the get template
int Decoding::get(T &val) {
  size_t blen = n_bits / 8;
  char buf[blen];
  size_t code = buf_.sgetn(buf, blen);
  if (code == blen) {
    val = load_little_endian<T, n_bits>(buf);
    return 0;
  } else {
    return -1;
  }
}
int Decoding::get(char *buf, size_t len) {
  size_t rlen = buf_.sgetn(buf, len);
  if (rlen == len) {
    return 0;
  } else {
    return -1;
  }
}
int Decoding::get(char *name, uint16_t &nlen) {
  if (get<uint16_t, 16>(nlen) != 0) {
    return -1;
  }
  size_t blen = buf_.sgetn(name, nlen);
  if (blen == nlen) {
    return 0;
  } else {
    return -1;
  }
}

int Decoding::get(char *name, uint16_t &nlen, char *value, uint16_t &vlen) {
  if (get(name, nlen) == 0 && get(value, vlen) == 0) {
    return 0;
  } else {
    return -1;
  }
}

Login::Login() {}

Login::Login(const char *hash, const char *name, uint32_t cid, uint16_t port,
             uint64_t version, uint64_t flags) {
  strcpy(this->hash, hash);
  strcpy(this->name, name);
  this->cid = cid;
  this->port = port;
  this->version = version;
  this->flags = flags;
}

Encoding &Login::encode() {
  enc.reset();
  enc.put(OP_LOGINREQUEST);
  enc.put(hash, (size_t)16);
  enc.put(cid);
  enc.put(port);
  enc.put((uint32_t)4);
  enc.put((uint8_t)0x1, name);
  enc.put((uint8_t)0x11, version);
  enc.put((uint8_t)0x0F, (uint64_t)port);
  enc.put((uint8_t)0x20, flags);
  return enc;
}

boost::asio::streambuf &Login::buf() { return encode().buf(); }

int Login::parse(boost::asio::streambuf &buf) {
  Decoding dec(buf);
  int code;
  uint8_t magic;
  if (dec.get<uint8_t, 8>(magic) != 0) {
    return -1;
  }
  if (magic != OP_LOGINREQUEST) {
    return 1;
  }
  if (dec.get(hash, 16) != 0) {
    return -1;
  }
  if (dec.get<uint32_t, 32>(cid) != 0) {
    return -1;
  }
  if (dec.get<uint16_t, 16>(port) != 0) {
    return -1;
  }
  uint32_t tc = 0;
  if (dec.get<uint32_t, 32>(tc) != 0) {
    return -1;
  }
  char nbuf[256];
  uint16_t nlen;
  char vbuf[256];
  uint16_t vlen;
  for (uint32_t i = 0; i < tc; i++) {
    if (dec.get<uint8_t, 8>(magic) != 0) {
      return -1;
    }
    if (dec.get(nbuf, nlen, vbuf, vlen) != 0) {
      return -1;
    }
    switch (nbuf[0]) {
    case 0x1:
      strncpy(name, vbuf, vlen);
      name[vlen] = 0;
      break;
    case 0x11:
      version = load_little_endian<uint64_t, 64>(vbuf);
      break;
    case 0x0F:
      port = load_little_endian<uint64_t, 64>(vbuf);
      break;
    case 0x20:
      flags = load_little_endian<uint64_t, 64>(vbuf);
      break;
    }
  }
  return 0;
}

void Login::print(char *buf) {
  char tbuf_[102400];
  char *tbuf = buf;
  if (tbuf == 0) {
    tbuf = tbuf_;
  }
  size_t tlen = 0;
  tlen += sprintf(tbuf + tlen, "%s", "<<Login>>\n Hash:");
  for (size_t i = 0; i < 16; i++) {
    tlen += sprintf(tbuf + tlen, "%2X ", (uint8_t)hash[i]);
  }
  tlen += sprintf(tbuf + tlen, "\n CID:%d", cid);
  tlen += sprintf(tbuf + tlen, "\n Port:%d", port);
  tlen += sprintf(tbuf + tlen, "\n Name:%s", name);
  tlen += sprintf(tbuf + tlen, "\n Version:0x%0llX", version);
  tlen += sprintf(tbuf + tlen, "\n Flags:0x%0llX", flags);
  tbuf[tlen] = 0;
  if (buf == 0) {
    printf("%s\n", tbuf);
  }
}
}
}
