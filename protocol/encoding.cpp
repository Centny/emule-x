//
//  encoding.cpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#include "encoding.hpp"

namespace emulex {
namespace protocol {
using namespace boost::endian::detail;
ModH BuildMod() { return ModH(new M1L4(PROTOCOL, false)); }
Encoding::Encoding() {}

void Encoding::reset() { buf_.consume(buf_.size()); }

// const char *Encoding::Encoding::data() { return asio::buffer_cast<const char *>(buf_.data()); }

size_t Encoding::Encoding::size() { return buf_.size(); }
boost::asio::streambuf &Encoding::buf() { return buf_; }
Data Encoding::Encoding::encode() {
    return BuildData(boost::asio::buffer_cast<const char *>(buf_.data()), buf_.size());
}

Encoding &Encoding::put(const char *val, size_t len) {
    buf_.sputn(val, len);
    return *this;
}
Encoding &Encoding::put(uint8_t val) {
    const char *data = endian::endian_buffer<endian::order::little, uint8_t, 8>(val).data();
    buf_.sputn(data, 1);
    return *this;
}
Encoding &Encoding::put(uint16_t val) {
    const char *data = endian::endian_buffer<endian::order::little, uint16_t, 16>(val).data();
    buf_.sputn(data, 2);
    return *this;
}
Encoding &Encoding::put(uint32_t val) {
    const char *data = endian::endian_buffer<endian::order::little, uint32_t, 32>(val).data();
    buf_.sputn(data, 4);
    return *this;
}
Encoding &Encoding::put(uint64_t val) {
    const char *data = endian::endian_buffer<endian::order::little, uint64_t, 64>(val).data();
    buf_.sputn(data, 8);
    return *this;
}
//    Encoding& Encoding::put(float val){
//      LittleEndianBuf::put<uint32_t, 32>(val);
//      return *this;
//    }
Encoding &Encoding::putv(const char *name, const char *val) {
    put(STR_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen);
    uint16_t vlen = strlen(val);
    put(vlen), put(val, vlen);
}
Encoding &Encoding::putv(uint8_t name, const char *val) {
    put(STR_TAG);
    put((uint16_t)1), put(name);
    uint16_t vlen = strlen(val);
    put(vlen), put(val, vlen);
}
Encoding &Encoding::putv(const char *name, uint32_t val) {
    put(INT_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen), put(val);
}
Encoding &Encoding::putv(uint8_t name, uint32_t val) {
    put(INT_TAG);
    put((uint16_t)1), put(name), put(val);
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

Encoding &Encoding::putv(const char *name, uint64_t val) {
    put(INT_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen), put(val);
}
Encoding &Encoding::putv(uint8_t name, uint64_t val) {
    put(INT_TAG);
    put((uint16_t)1), put(name), put(val);
}

void Encoding::print(char *buf) {
    Data data = encode();
    size_t len = data->len;
    char tbuf_[102400];
    char *tbuf = buf;
    if (tbuf == 0) {
        tbuf = tbuf_;
    }
    size_t tlen = sprintf(tbuf, "ENC(%ld):", len);
    for (size_t i = 0; i < len; i++) {
        tlen += sprintf(tbuf + tlen, "%2x ", (uint8_t)data->data[i]);
    }
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
    }
}

Decoding::Decoding(Data &data) {
    this->data = data->share();
    this->offset = 0;
}
Decoding::~Decoding() {}
int Decoding::get(char *buf, size_t len) {
    if (data->len < offset + len) {
        return -1;
    }
    memcpy(buf, data->data + offset, len);
    offset += len;
    return 0;
}
int Decoding::get(char *name, uint16_t &nlen) {
    if (get<uint16_t, 2>(nlen) != 0) {
        return -1;
    }
    return get(name, (size_t)nlen);
}

int Decoding::get(char *name, uint16_t &nlen, char *value, uint16_t &vlen) {
    if (get(name, nlen) == 0 && get(value, vlen) == 0) {
        return 0;
    } else {
        return -1;
    }
}
int Decoding::get(char *name, uint16_t &nlen, uint32_t &vlen) {
    if (get(name, nlen) == 0 && get<uint32_t, 4>(vlen) == 0) {
        return 0;
    } else {
        return -1;
    }
}

Login::Login() {
    memset(hash, 0, 16);
    memset(name, 0, 256);
    cid = 0;
    port = 0;
    version = 0;
    flags = 0;
}

Login::Login(const char *hash, const char *name, uint32_t cid, uint16_t port, uint64_t version, uint64_t flags) {
    memset(this->hash, 0, 16);
    memcpy(this->hash, hash, 16);
    memset(this->name, 0, 256);
    strcpy(this->name, name);
    this->cid = cid;
    this->port = port;
    this->version = version;
    this->flags = flags;
}

Encoding &Login::encoder() {
    enc.reset();
    enc.put(OP_LOGINREQUEST);
    enc.put(hash, (size_t)16);
    enc.put(cid);
    enc.put(port);
    enc.put((uint32_t)4);
    enc.putv((uint8_t)0x1, name);
    enc.putv((uint8_t)0x11, version);
    enc.putv((uint8_t)0x0F, (uint64_t)port);
    enc.putv((uint8_t)0x20, flags);
    return enc;
}

Data Login::encode() { return encoder().encode(); }

int Login::parse(Data &data) {
    Decoding dec(data);
    int code;
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_LOGINREQUEST) {
        return 1;
    }
    if (dec.get(hash, 16) != 0) {
        return -1;
    }
    if (dec.get<uint32_t, 4>(cid) != 0) {
        return -1;
    }
    if (dec.get<uint16_t, 2>(port) != 0) {
        return -1;
    }
    uint32_t tc = 0;
    if (dec.get<uint32_t, 4>(tc) != 0) {
        return -1;
    }
    char nbuf[256];
    uint16_t nlen;
    char vbuf[256];
    uint16_t vlen;
    uint32_t ival;
    for (uint32_t i = 0; i < tc; i++) {
        if (dec.get<uint8_t, 1>(magic) != 0) {
            return -1;
        }
        if (magic == STR_TAG) {
            if (dec.get(nbuf, nlen, vbuf, vlen) != 0) {
                return -1;
            }
            switch (nbuf[0]) {
                case 0x1:
                    memcpy(name, vbuf, vlen);
                    name[vlen] = 0;
                    break;
            }
        } else if (magic == INT_TAG) {
            if (dec.get(nbuf, nlen, ival) != 0) {
                return -1;
            }
            switch (nbuf[0]) {
                case 0x11:
                    version = ival;
                    break;
                case 0x0F:
                    port2 = ival;
                    break;
                case 0x20:
                    flags = ival;
                    break;
            }
        } else {
            continue;
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
