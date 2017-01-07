//
//  encoding.cpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#include "encoding.hpp"
#include <zlib.h>
namespace emulex {
namespace protocol {
using namespace boost::endian::detail;

ModH BuildMod() {
    auto mod = ModH(new M1L4());
    M1L4 *m = (M1L4 *)mod.get();
    m->magic[0] = OP_EDONKEYPROT;
    m->magic[1] = OP_PACKEDPROT;
    m->big = false;
    return mod;
}

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
void Decoding::get(char *buf, size_t len) {
    if (data->len < offset + len) {
        throw Fail("decode fail with not enough data, expect %ld, but %ld", len, data->len - offset);
    }
    memcpy(buf, data->data + offset, len);
    offset += len;
}
void Decoding::get(char *name, uint16_t &nlen) {
    nlen = get<uint16_t, 2>();
    get(name, (size_t)nlen);
}

void Decoding::get(char *name, uint16_t &nlen, char *value, uint16_t &vlen) {
    get(name, nlen);
    get(value, vlen);
}
void Decoding::get(char *name, uint16_t &nlen, uint32_t &vlen) {
    get(name, nlen);
    vlen = get<uint32_t, 4>();
}
Data Decoding::getstring(std::size_t len) {
    if (len) {
        return getdata(len, true);
    } else {
        return getdata(get<uint16_t, 2>(), true);
    }
}
Data Decoding::getdata(size_t len, bool iss) {
    Data val = BuildData(len, iss);
    get(val->data, val->len);
    return val;
}

void Decoding::inflate() {
    if (data->inflate(offset)) {
        throw Fail("Decoding inflate fail with offset(%ld)", offset);
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
    put((uint8_t)OP_LOGINREQUEST);
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

void Login::parse(Data &data) {
    Decoding dec(data);
    int code;
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_LOGINREQUEST) {
        throw Fail("Login parse fail with invalid magic, %x expected, but %x", OP_LOGINREQUEST, magic);
    }
    dec.get(hash, 16);
    cid = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
    uint32_t tc = dec.get<uint32_t, 4>();
    uint16_t nlen, vlen;
    char nbuf[256], vbuf[256];
    uint32_t ival;
    for (uint32_t i = 0; i < tc; i++) {
        magic = dec.get<uint8_t, 1>();
        if (magic == STR_TAG) {
            dec.get(nbuf, nlen, vbuf, vlen);
            switch ((unsigned char)nbuf[0]) {
                case 0x1:
                    memcpy(name, vbuf, vlen);
                    name[vlen] = 0;
                    break;
            }
        } else if (magic == INT_TAG) {
            dec.get(nbuf, nlen, ival);
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
    put((uint8_t)OP_SERVERMESSAGE);
    put((uint16_t)msg->len);
    put(msg);
    return Encoding::encode();
}
void SrvMessage::parse(Data &data) {
    Decoding dec(data);
    uint16_t len;
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_SERVERMESSAGE) {
        throw Fail("SrvMessage parse fail with invalid magic, %x expected, but %x", OP_SERVERMESSAGE, magic);
    }
    len = dec.get<uint16_t, 2>();
    if (data->len < len + 2) {
        throw Fail("SrvMessage parse fail with need more data, %x expected, but %x", len, data->len - 2);
    }
    msg = data->sub(3, len);
}
const char *SrvMessage::c_str() { return std::string(msg->data, msg->len).c_str(); }

IDCHANGE::IDCHANGE(uint32_t id, uint32_t bitmap) {
    this->id = id;
    this->bitmap = bitmap;
}

Data IDCHANGE::encode() {
    reset();
    put((uint8_t)OP_IDCHANGE);
    put(id);
    put(bitmap);
    return Encoding::encode();
}

void IDCHANGE::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_IDCHANGE) {
        throw Fail("IDCHANGE parse fail with invalid magic, %x expected, but %x", OP_IDCHANGE, magic);
    }
    id = dec.get<uint32_t, 4>();
    bitmap = dec.get<uint32_t, 4>();
}

SrvStatus::SrvStatus(uint32_t userc, uint32_t filec) {
    this->userc = userc;
    this->filec = filec;
}

Data SrvStatus::encode() {
    reset();
    put((uint8_t)OP_SERVERSTATUS);
    put(userc);
    put(filec);
    return Encoding::encode();
}

void SrvStatus::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_SERVERSTATUS) {
        throw Fail("SrvStatus parse fail with invalid magic, %x expected, but %x", OP_SERVERSTATUS, magic);
    }
    userc = dec.get<uint32_t, 4>();
    filec = dec.get<uint32_t, 4>();
}

void FTagParser::parse(Decoding &dec) {
    uint16_t length;
    type = dec.get<uint8_t, 1>();
    if (type & 0x80) {
        type &= 0x7F;
        uname = dec.get<uint8_t, 1>();
    } else {
        length = dec.get<uint16_t, 2>();
        printf("->%ld\n", length);
        if (length == 1) {
            uname = dec.get<uint8_t, 1>();
        } else {
            uname = 0;
            sname = dec.getstring(length);
        }
    }
    // NOTE: It's very important that we read the *entire* packet data, even if we do
    // not use each tag. Otherwise we will get troubles when the packets are returned in
    // a list - like the search results from a server.
    if (type == TAGTYPE_STRING) {
        sval = dec.getstring();
    } else if (type == TAGTYPE_UINT32) {
        u32v = dec.get<uint32_t, 4>();
    } else if (type == TAGTYPE_UINT64) {
        u64v = dec.get<uint64_t, 8>();
    } else if (type == TAGTYPE_UINT16) {
        u16v = dec.get<uint16_t, 2>();
    } else if (type == TAGTYPE_UINT8) {
        u8v = dec.get<uint8_t, 1>();
    } else if (type == TAGTYPE_FLOAT32) {
        //        data->Read(&m_fVal, 4);
    } else if (type >= TAGTYPE_STR1 && type <= TAGTYPE_STR16) {
        sval = dec.getstring(type - TAGTYPE_STR1 + 1);
        type = TAGTYPE_STRING;
    } else if (type == TAGTYPE_HASH) {
        sval = dec.getdata(16);
    } else if (type == TAGTYPE_BOOL) {
    } else if (type == TAGTYPE_BOOLARRAY) {
    } else if (type == TAGTYPE_BLOB) {
        sval = dec.getdata(dec.get<uint32_t, 4>());
    } else {
        throw Fail("FTagParser Unknown tag: type=0x%02X  specialtag=%u\n", __FUNCTION__, type, uname);
    }
}

FileEntry_::~FileEntry_() { V_LOG_FREE("%s", "FileEntry_ free"); }

void FileEntry_::parse(Decoding &dec, uint8_t magic) {
    dec.get(hash, 16);
    cid = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
    uint32_t tc = dec.get<uint32_t, 4>();
    for (uint32_t i = 0; i < tc; i++) {
        FTagParser tag;
        tag.parse(dec);
        if (tag.uname) {
            if (tag.uname == FT_FILENAME && tag.sval.get()) {
                name = tag.sval->share();
            } else if (tag.uname == FT_FILESIZE) {
                size = tag.u64v;
            } else if (tag.uname == FT_FILETYPE && tag.sval.get()) {
                type = tag.sval->share();
            } else if (tag.uname == FT_FILEFORMAT && tag.sval.get()) {
                format = tag.sval->share();
            } else if (tag.uname == FT_SOURCES) {
                sources = tag.u8v;
            } else if (tag.uname == FT_COMPLETE_SOURCES) {
                completed = tag.u8v;
            } else if (tag.uname == FT_GAPSTART) {
                gapstart = tag.u32v;
            } else {
                V_LOG_W("FileEntry found unknow tag(%02X) name(%02X)", tag.type, tag.uname);
            }
        } else if (tag.sname.get()) {
            if (tag.sname->cmp("Artist") && tag.sval.get()) {
                artist = tag.sval->share();
            } else if (tag.sname->cmp("Album") && tag.sval.get()) {
                album = tag.sval->share();
            } else if (tag.sval->cmp("Title")) {
                title = tag.sval->share();
            } else if (tag.sval->cmp("length")) {
                length = tag.u32v;
            } else if (tag.sval->cmp("bitrate")) {
                bitrate = tag.u32v;
            } else if (tag.sval->cmp("codec")) {
                codec = tag.u32v;
            } else {
                V_LOG_W("FileEntry found unknow tag:%02X", tag.type);
            }
        } else {
            V_LOG_W("FileEntry found unknow tag:%ld", tag.type);
        }
    }
}

void FileEntry_::print() {
    if (type.get()) {
        printf("%s,%llu,%s\n", name->data, size, type->data);
    } else if (format.get()) {
        printf("%s,%llu,%s\n", name->data, size, format->data);
    } else {
        printf("%s,%llu,%u,%u\n", name->data, size, sources, completed);
    }
}

std::string FileEntry_::shash() {
    char buf[33];
    for (int i = 0; i < 16; i++) {
        sprintf(buf + i * 2, "%02X", hash[i]);
    }
    return std::string(buf);
}

FileEntry FileEntry_::share() { return shared_from_this(); }

FileEntry BuildFileEntry() { return FileEntry(new FileEntry_()); }

FileList::FileList() {}

Data FileList::encode() { return Encoding::encode(); }

void FileList::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t magicx = dec.get<uint8_t, 1>();
    if (magicx != OP_SEARCHRESULT && magicx != OP_OFFERFILES) {
        throw Fail("FileList parse fail with invalid magic, %x or %x expected, but %x", OP_SERVERIDENT, OP_OFFERFILES,
                   magicx);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    uint32_t rc = dec.get<uint32_t, 4>();
    for (uint32_t i = 0; i < rc; i++) {
        FileEntry fe = BuildFileEntry();
        fe->parse(dec, magic);
        fe->print();
        this->fs[fe->shash()] = fe->share();
    }
}

Data ListServer() {
    char opcode = (char)OP_SERVERSTATUS;
    return BuildData(&opcode, 1);
}

ServerList::ServerList() {}

Data ServerList::encode() {
    reset();
    put((uint8_t)OP_SERVERLIST);
    BOOST_FOREACH (Address &srv, srvs) {
        put(srv.first);
        put(srv.second);
    }
    return Encoding::encode();
}

void ServerList::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_SERVERLIST) {
        throw Fail("ServerList parse fail with invalid magic, %x expected, but %x", OP_SERVERLIST, magic);
    }
    uint8_t count = dec.get<uint8_t, 1>();
    uint32_t ip;
    uint16_t port;
    for (uint8_t i = 0; i < count; i++) {
        ip = dec.get<uint32_t, 4>();
        port = dec.get<uint16_t, 2>();
        srvs.push_back(Address(ip, port));
    }
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
    put((uint8_t)OP_SERVERIDENT);
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

void ServerIndent::parse(Data &data) {
    Decoding dec(data);
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_SERVERIDENT) {
        throw Fail("ServerIndent parse fail with invalid magic, %x expected, but %x", OP_SERVERIDENT, magic);
    }
    dec.get(hash, 16);
    ip = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
    uint32_t tc = dec.get<uint32_t, 4>();
    uint16_t nlen, vlen;
    char nbuf[256], vbuf[256];
    uint32_t ival;
    for (uint32_t i = 0; i < tc; i++) {
        magic = dec.get<uint8_t, 1>();
        if (magic != STR_TAG) {
            throw Fail("ServerIndent parse fail with invalid magic, %x expected, but %x", STR_TAG, magic);
        }
        dec.get(nbuf, nlen, vbuf, vlen);
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
}

SearchArgs::SearchArgs() {}

SearchArgs::SearchArgs(const char *search) { this->search = BuildData(search, strlen(search)); }

Data SearchArgs::encode() {
    reset();
    put((uint8_t)OP_SEARCHREQUEST);
    //        put((uint16_t)0);
    //        put(search->len);
    put((uint8_t)0x1);
    put((uint16_t)search->len);
    put(search->data, search->len);
    return Encoding::encode();
}

void SearchArgs::parse(Data &data) {
    Decoding dec(data);
    int code;
    uint8_t magic = dec.get<uint8_t, 1>();
    if (magic != OP_SEARCHREQUEST) {
        throw Fail("ServerIndent parse fail with invalid magic, %x expected, but %x", OP_SEARCHREQUEST, magic);
    }
    search = data->sub(1, data->len - 1);
    return 0;
}

//////////end encoding//////////
}
}
