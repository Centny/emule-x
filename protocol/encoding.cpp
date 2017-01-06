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

Encoding &Encoding::put(Data &data) {
    put(data->data, data->len);
    return *this;
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

Login::Login(const char *hash, const char *name, uint32_t cid, uint16_t port, uint32_t version, uint32_t flags) {
    memset(this->hash, 0, 22);
    memcpy(this->hash, hash, 22);
    memset(this->name, 0, 256);
    strcpy(this->name, name);
    this->cid = cid;
    this->port = port;
    this->version = version;
    this->flags = flags;
}

Data Login::encode() {
    reset();
    put(OP_LOGINREQUEST);
    put(hash, (size_t)16);
    put(cid);
    put(port);
    put((uint32_t)4);
    putv((uint8_t)0x1, name);
    putv((uint8_t)0x11, version);
    //    putv((uint8_t)0x0F, (uint32_t)port);
    putv((uint8_t)0x20, flags);
    other = 0 << 17 | 50 << 10 | 0 << 7;
    putv((uint8_t)0xfb, other);
    return Encoding::encode();
}

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
            switch ((unsigned char)nbuf[0]) {
                case 0x1:
                    memcpy(name, vbuf, vlen);
                    name[vlen] = 0;
                    break;
            }
        } else if (magic == INT_TAG) {
            if (dec.get(nbuf, nlen, ival) != 0) {
                return -1;
            }
            switch ((unsigned char)nbuf[0]) {
                case 0x11:
                    version = ival;
                    break;
                case 0x0F:
                    port2 = ival;
                    break;
                case 0x20:
                    flags = ival;
                    break;
                case 0xfb:
                    other = ival;
                    break;
            }
        } else {
            continue;
        }
    }
    return 0;
}
size_t Login::show(char *buf) {
    char tbuf[512];
    size_t tlen = 0;
    tlen += sprintf(tbuf + tlen, "\n CID:%d", cid);
    tlen += sprintf(tbuf + tlen, "\n Port:%d", port);
    tlen += sprintf(tbuf + tlen, "\n Name:%s", name);
    tlen += sprintf(tbuf + tlen, "\n Version:%0x", version);
    tlen += sprintf(tbuf + tlen, "\n Flags:%0x", flags);
    tlen += sprintf(tbuf + tlen, "\n Other:%0x", other);
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
    }
    return tlen;
}
SrvMessage::SrvMessage() {}
SrvMessage::SrvMessage(const char *msg, size_t len) { this->msg = BuildData(msg, len); }

Data SrvMessage::encode() {
    reset();
    put(OP_SERVERMESSAGE);
    put((uint16_t)msg->len);
    put(msg);
    return Encoding::encode();
}
int SrvMessage::parse(Data &data) {
    Decoding dec(data);
    uint16_t len;
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_SERVERMESSAGE) {
        return 1;
    }
    if (dec.get<uint16_t, 2>(len) != 0) {
        return -1;
    }
    if (data->len < len + 2) {
        return -1;
    }
    msg = data->sub(3, len);
    return 0;
}
const char *SrvMessage::c_str() { return std::string(msg->data, msg->len).c_str(); }

IDCHANGE::IDCHANGE(uint32_t id, uint32_t bitmap) {
    this->id = id;
    this->bitmap = bitmap;
}

Data IDCHANGE::encode() {
    reset();
    put(OP_IDCHANGE);
    put(id);
    put(bitmap);
    return Encoding::encode();
}

int IDCHANGE::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_IDCHANGE) {
        return 1;
    }
    if (dec.get<uint32_t, 4>(id) != 0) {
        return -1;
    }
    return dec.get<uint32_t, 4>(bitmap);
}

SrvStatus::SrvStatus(uint32_t userc, uint32_t filec) {
    this->userc = userc;
    this->filec = filec;
}

Data SrvStatus::encode() {
    reset();
    put(OP_SERVERSTATUS);
    put(userc);
    put(filec);
    return Encoding::encode();
}

int SrvStatus::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_SERVERSTATUS) {
        return 1;
    }
    if (dec.get<uint32_t, 4>(userc) != 0) {
        return -1;
    }
    return dec.get<uint32_t, 4>(filec);
}

OfferFile::OfferFile() {}

Data OfferFile::encode() { return Encoding::encode(); }

int OfferFile::parse(Data &data) { return 0; }

Data ListServer() {
    char opcode = (char)OP_LIST_SERVER;
    return BuildData(&opcode, 1);
}

ServerList::ServerList() {}

Data ServerList::encode() {
    reset();
    put(OP_SERVERLIST);
    BOOST_FOREACH (Address &srv, srvs) {
        put(srv.first);
        put(srv.second);
    }
    return Encoding::encode();
}

int ServerList::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_SERVERLIST) {
        return 1;
    }
    uint8_t count;
    if (dec.get<uint8_t, 1>(count) != 0) {
        return -1;
    }
    uint32_t ip;
    uint16_t port;
    for (uint8_t i = 0; i < count; i++) {
        if (dec.get<uint32_t, 4>(ip) != 0) {
            return -1;
        }
        if (dec.get<uint16_t, 2>(port) != 0) {
            return -1;
        }
        srvs.push_back(Address(ip, port));
    }
    return 0;
}

ServerIndent::ServerIndent() {
    memset(this->hash, 0, 16);
    this->ip = 0;
    this->port = 0;
    memset(this->name, 0, 128);
    memset(this->desc, 0, 256);
}

ServerIndent::ServerIndent(const char *hash, uint32_t ip, uint32_t port, const char *name, const char *desc) {
    memcpy(this->hash, hash, 16);
    this->ip = ip;
    this->port = port;
    strncpy(this->name, name, 128);
    strncpy(this->desc, desc, 256);
}

Data ServerIndent::encode() {
    reset();
    put(OP_SERVERIDENT);
    put(hash, 16);
    put(ip);
    put(port);
    uint32_t tc = 0;
    if (strlen(name)) {
        tc++;
    }
    if (strlen(desc)) {
        tc++;
    }
    if (tc) {
        put(tc);
        if (strlen(name)) {
            putv(uint8_t(0x1), name);
        }
        if (strlen(desc)) {
            putv(uint8_t(0x1), desc);
        }
    }
    return Encoding::encode();
}

int ServerIndent::parse(Data &data) {
    Decoding dec(data);
    int code;
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_SERVERIDENT) {
        return 1;
    }
    if (dec.get(hash, 16) != 0) {
        return -1;
    }
    if (dec.get<uint32_t, 4>(ip) != 0) {
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
        if (magic != STR_TAG) {
            return -1;
        }
        if (dec.get(nbuf, nlen, vbuf, vlen) != 0) {
            return -1;
        }
        switch ((unsigned char)nbuf[0]) {
            case 0x1:
                memcpy(name, vbuf, vlen);
                name[vlen] = 0;
                break;
            case 0xB:
                memcpy(desc, vbuf, vlen);
                desc[vlen] = 0;
                break;
        }
    }
    return 0;
}

SearchArgs::SearchArgs() {}

SearchArgs::SearchArgs(const char *search) { this->search = BuildData(search, strlen(search)); }

Data SearchArgs::encode() {
    reset();
    put(OP_SEARCHREQUEST);
    //        put((uint16_t)0);
    //        put(search->len);
    put((uint8_t)0x1);
    put((uint16_t)search->len);
    put(search->data, search->len);
    return Encoding::encode();
}

int SearchArgs::parse(Data &data) {
    Decoding dec(data);
    int code;
    uint8_t magic;
    if (dec.get<uint8_t, 1>(magic) != 0) {
        return -1;
    }
    if (magic != OP_SEARCHREQUEST) {
        return 1;
    }
    search = data->sub(1, data->len - 1);
    return 0;
}

//////////end encoding//////////
}
}
