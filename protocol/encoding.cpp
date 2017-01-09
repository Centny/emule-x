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

Hash::Hash() {}

Hash::Hash(size_t len) : Data(new Data_(len, false)) {}

Hash::Hash(const char *buf, size_t len) : Data(new Data_(buf, len)) {}

Hash::~Hash() {
    // V_LOG_FREE("%s", "Hash_ free...");
}

void Hash::set(size_t len) { this->reset(new Data_(len, false)); }

void Hash::set(const char *buf, size_t len) { this->reset(new Data_(buf, len, false)); }

bool Hash::operator==(const Hash &h) const {
    Data_ *a = get();
    Data_ *b = h.get();
    if (a == 0 || b == 0) {
        return false;
    }
    if (a->len != b->len) {
        return false;
    }
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) {
            return false;
        }
    }
    return true;
}

bool Hash::operator<(const Hash &h) const {
    Data_ *a = get();
    Data_ *b = h.get();
    if (a == 0 || b == 0) {
        return true;
    }
    if (a->len != b->len) {
        return true;
    }
    for (size_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) {
            return a->data[i] < b->data[i];
        }
    }
    return false;
}

std::string Hash::tostring() {
    Data_ *a = get();
    return hash_tos(a->data);
}

std::string hash_tos(const char *hash) {
    char buf[33];
    for (int i = 0; i < 16; i++) {
        sprintf(buf + i * 2, "%02X", hash[i]);
    }
    return std::string(buf);
}

Hash BuildHash(size_t len) { return Hash(len); }

Hash BuildHash(const char *buf, size_t len) { return Hash(buf, len); }

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
Encoding &Encoding::put(uint8_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint8_t, 8>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint8_t, 8>(val).data();
    }
    buf_.sputn(data, 1);
    return *this;
}
Encoding &Encoding::put(uint16_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint16_t, 16>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint16_t, 16>(val).data();
    }
    buf_.sputn(data, 2);
    return *this;
}
Encoding &Encoding::put(uint32_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint32_t, 32>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint32_t, 32>(val).data();
    }
    buf_.sputn(data, 4);
    return *this;
}
Encoding &Encoding::put(uint64_t val, bool big) {
    const char *data;
    if (big) {
        data = endian::endian_buffer<endian::order::big, uint64_t, 64>(val).data();
    } else {
        data = endian::endian_buffer<endian::order::little, uint64_t, 64>(val).data();
    }
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
Encoding &Encoding::putv(Data &name, Data &val) {
    put(STR_TAG);
    put((uint16_t)name->len), put(name->data, name->len);
    put((uint16_t)val->len), put(val->data, val->len);
}
Encoding &Encoding::putv(uint8_t name, const char *val) {
    put(STR_TAG);
    put((uint16_t)1), put(name);
    uint16_t vlen = strlen(val);
    put(vlen), put(val, vlen);
}
Encoding &Encoding::putv(const char *name, Data &val) {
    put(STR_TAG);
    uint16_t nlen = strlen(name);
    put(nlen), put(name, nlen);
    put((uint16_t)val->len), put(val->data, val->len);
}
Encoding &Encoding::putv(uint8_t name, Data &val) {
    put(STR_TAG);
    put((uint16_t)1), put(name);
    put((uint16_t)val->len), put(val->data, val->len);
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

Login::Login() : hash(16) {
    cid = 0;
    port = 0;
    version = 0;
    flags = 0;
    mver = 0 << 17 | 50 << 10 | 0 << 7;
}

Login::Login(Hash &hash, Data &name, uint32_t cid, uint16_t port, uint32_t version, uint32_t flags) {
    this->hash = hash;
    this->name = name;
    this->cid = cid;
    this->port = port;
    this->version = version;
    this->flags = flags;
    mver = 0 << 17 | 50 << 10 | 0 << 7;
}

Data Login::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_LOGINREQUEST);
    enc->put(hash->data, hash->len);
    enc->put(cid);
    enc->put(port);
    enc->put((uint32_t)4);
    enc->putv((uint8_t)0x1, name);
    enc->putv((uint8_t)0x11, version);
    //    putv((uint8_t)0x0F, (uint32_t)port);
    enc->putv((uint8_t)0x20, flags);
    enc->putv((uint8_t)0xfb, mver);
    auto data = enc->encode();
    delete enc;
    return data;
}

void Login::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    int code;
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_LOGINREQUEST) {
        throw Fail("Login parse fail with invalid magic, %x expected, but %x", OP_LOGINREQUEST, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
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
                    name.reset(new Data_(vbuf, vlen, true));
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
                    mver = ival;
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
    tlen += sprintf(tbuf + tlen, "\n Name:%s", name->data);
    tlen += sprintf(tbuf + tlen, "\n Version:%0x", version);
    tlen += sprintf(tbuf + tlen, "\n Flags:%0x", flags);
    tlen += sprintf(tbuf + tlen, "\n MVer:%0x", mver);
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
    }
    return tlen;
}

SrvMessage::SrvMessage() {}

SrvMessage::SrvMessage(const char *msg, size_t len) { this->msg = BuildData(msg, len); }

Data SrvMessage::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SERVERMESSAGE);
    enc->put((uint16_t)msg->len);
    enc->put(msg);
    auto data = enc->encode();
    delete enc;
    return data;
}

void SrvMessage::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_SERVERMESSAGE) {
        throw Fail("SrvMessage parse fail with invalid magic, %x expected, but %x", OP_SERVERMESSAGE, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    uint16_t len = dec.get<uint16_t, 2>();
    if (data->len < len + 3) {
        throw Fail("SrvMessage parse fail with need more data, %x expected, but %x", len, data->len - 3);
    }
    msg = data->sub(3, len, true);
}

const char *SrvMessage::c_str() { return std::string(msg->data, msg->len).c_str(); }

IDCHANGE::IDCHANGE(uint32_t id, uint32_t bitmap) {
    this->id = id;
    this->bitmap = bitmap;
}

Data IDCHANGE::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_IDCHANGE);
    enc->put(id);
    enc->put(bitmap);
    auto data = enc->encode();
    delete enc;
    return data;
}

void IDCHANGE::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_IDCHANGE) {
        throw Fail("IDCHANGE parse fail with invalid magic, %x expected, but %x", OP_IDCHANGE, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    id = dec.get<uint32_t, 4>();
    bitmap = dec.get<uint32_t, 4>();
}

SrvStatus::SrvStatus(uint32_t userc, uint32_t filec) {
    this->userc = userc;
    this->filec = filec;
}

Data SrvStatus::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SERVERSTATUS);
    enc->put(userc);
    enc->put(filec);
    auto data = enc->encode();
    delete enc;
    return data;
}

void SrvStatus::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_SERVERSTATUS) {
        throw Fail("SrvStatus parse fail with invalid magic, %x expected, but %x", OP_SERVERSTATUS, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
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
        u64v = u32v = dec.get<uint32_t, 4>();
    } else if (type == TAGTYPE_UINT64) {
        u64v = dec.get<uint64_t, 8>();
    } else if (type == TAGTYPE_UINT16) {
        u64v = u32v = u16v = dec.get<uint16_t, 2>();
    } else if (type == TAGTYPE_UINT8) {
        u64v = u32v = u16v = u8v = dec.get<uint8_t, 1>();
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

FileEntry_::FileEntry_() : hash(16) {}

FileEntry_::~FileEntry_() {
    // V_LOG_FREE("%s", "FileEntry_ free");
}

void FileEntry_::parse(Decoding &dec, uint8_t magic) {
    dec.get(hash->data, hash->len);
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
            } else if (tag.uname == FT_FILETYPE) {
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
                V_LOG_W("FileEntry found unknow tag(%02X) name(%s)", tag.type, tag.sname->data);
            }
        } else {
            V_LOG_W("FileEntry found unknow tag:%ld", tag.type);
        }
    }
}

void FileEntry_::print() {
    printf("%s,%s,%llu,%u,%u\n", hash.tostring().c_str(), name->data, size, sources, completed);
}

std::string FileEntry_::shash() { return hash.tostring(); }

FileEntry FileEntry_::share() { return shared_from_this(); }

FileEntry BuildFileEntry() { return FileEntry(new FileEntry_()); }

FileList::FileList() {}

Data FileList::encode() {
    auto enc = new Encoding();
    auto data = enc->encode();
    delete enc;
    return data;
}

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
        //        fe->print();
        fs.push_back(fe);
    }
}

Data ListServer() {
    char opcode = (char)OP_SERVERSTATUS;
    return BuildData(&opcode, 1);
}
std::string addr_cs(Address &addr) {
    std::stringstream ss;
    ss << boost::asio::ip::address_v4(addr.first).to_string();
    ss << ":" << addr.second;
    return ss.str();
}
ServerList::ServerList() {}

Data ServerList::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SERVERLIST);
    BOOST_FOREACH (Address &srv, srvs) {
        enc->put(srv.first);
        enc->put(srv.second);
    }
    auto data = enc->encode();
    delete enc;
    return data;
}

void ServerList::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_SERVERLIST) {
        throw Fail("ServerList parse fail with invalid magic, %x expected, but %x", OP_SERVERLIST, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
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

ServerIndent::ServerIndent() {}

ServerIndent::ServerIndent(Hash &hash, uint32_t addr, uint32_t port, Data &name, Data &desc) {
    this->hash = hash;
    this->addr = addr;
    this->port = port;
    this->name = name;
    this->desc = desc;
}

Data ServerIndent::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SERVERIDENT);
    enc->put(hash->data, hash->len);
    enc->put(addr, true);
    enc->put(port);
    uint32_t tc = 0;
    if (name) {
        tc++;
    }
    if (desc) {
        tc++;
    }
    if (tc) {
        enc->put(tc);
        if (name) {
            enc->putv(uint8_t(0x1), name);
        }
        if (desc) {
            enc->putv(uint8_t(0x1), desc);
        }
    }
    auto data = enc->encode();
    delete enc;
    return data;
}

void ServerIndent::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_SERVERIDENT) {
        throw Fail("ServerIndent parse fail with invalid magic, %x expected, but %x", OP_SERVERIDENT, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
    addr = dec.get<uint32_t, 4>();
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
                name.reset(new Data_(vbuf, vlen, true));
                break;
            case 0xB:
                desc.reset(new Data_(vbuf, vlen, true));
                break;
        }
    }
}

std::string ServerIndent::tostr() {
    std::stringstream ss;
    ss << boost::asio::ip::address_v4(addr) << ":" << port;
    if (name) {
        ss << "," << name->data;
    }
    if (desc) {
        ss << "," << desc->data;
    }
    return ss.str();
}

SearchArgs::SearchArgs() {}

SearchArgs::SearchArgs(const char *search) { this->search = BuildData(search, strlen(search)); }

Data SearchArgs::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SEARCHREQUEST);
    //        put((uint16_t)0);
    //        put(search->len);
    enc->put((uint8_t)0x1);
    enc->put((uint16_t)search->len);
    enc->put(search->data, search->len);
    auto data = enc->encode();
    delete enc;
    return data;
}

void SearchArgs::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (magic != OP_SEARCHREQUEST) {
        throw Fail("ServerIndent parse fail with invalid magic, %x expected, but %x", OP_SEARCHREQUEST, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    search = data->sub(1, data->len - 1);
    return 0;
}

GetSource::GetSource() : hash(16) {}

GetSource::GetSource(Hash &hash, uint64_t size) {
    this->hash = hash;
    this->size = size;
}

Data GetSource::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_GETSOURCES_OBFU);
    enc->put(hash->data, hash->len);
    if (size > (uint64_t)OLD_MAX_EMULE_FILE_SIZE) {
        enc->put((uint32_t)0);
        enc->put((uint64_t)size);
    } else {
        enc->put((uint32_t)size);
    }
    auto data = enc->encode();
    delete enc;
    return data;
}

void GetSource::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_GETSOURCES_OBFU) {
        throw Fail("GetSource parse fail with invalid magic, %x expected, but %x", OP_GETSOURCES_OBFU, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
    size = dec.get<uint32_t, 4>();
    if (size == 0) {
        size = dec.get<uint64_t, 8>();
    }
}

FoundSource::FoundSource() : hash(16) {}

Data FoundSource::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_FOUNDSOURCES_OBFU);
    //        put((uint16_t)0);
    //        put(search->len);
    enc->put(hash->data, hash->len);
    enc->put((uint8_t)srvs.size());
    BOOST_FOREACH (Address &srv, srvs) {
        enc->put(srv.first, true);
        enc->put(srv.second);
        enc->put((uint8_t)0x01);
    }
    auto data = enc->encode();
    delete enc;
    return data;
}

void FoundSource::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_FOUNDSOURCES_OBFU) {
        throw Fail("FoundSource parse fail with invalid magic, %x expected, but %x", OP_FOUNDSOURCES_OBFU, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
    uint8_t count = dec.get<uint8_t, 1>();
    uint32_t addr;
    uint16_t port;
    for (uint8_t i = 0; i < count; i++) {
        addr = dec.get<uint32_t, 4>(true);
        port = dec.get<uint16_t, 2>();
        srvs.push_back(Address(addr, port));
        dec.get<uint8_t, 1>();
    }
}

CallbackRequest::CallbackRequest(uint32_t cid) { this->cid = cid; }

Data CallbackRequest::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_CALLBACKREQUEST);
    enc->put(cid);
    auto data = enc->encode();
    delete enc;
    return data;
}

void CallbackRequest::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_CALLBACKREQUEST) {
        throw Fail("CallbackRequest parse fail with invalid magic, %x expected, but %x", OP_CALLBACKREQUEST, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    cid = dec.get<uint32_t, 4>();
}

CallbackRequested::CallbackRequested() {}

Data CallbackRequested::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_CALLBACKREQUESTED);
    enc->put((uint32_t)first);
    enc->put((uint16_t)second);
    auto data = enc->encode();
    delete enc;
    return data;
}
void CallbackRequested::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_CALLBACKREQUESTED) {
        throw Fail("CallbackRequested parse fail with invalid magic, %x expected, but %x", OP_CALLBACKREQUESTED,
                   xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    first = dec.get<uint32_t, 4>();
    second = dec.get<uint16_t, 2>();
}

Hello::Hello(uint8_t magic) { this->magic = magic; }

Data Hello::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_HELLO);
    enc->put((uint8_t)hash->len);
    enc->put(hash->data, hash->len);
    enc->put((uint32_t)cid);
    enc->put((uint16_t)port);
    uint32_t tc = 0;
    if (name) {
        tc++;
    }
    if (version) {
        tc++;
    }
    if (port2) {
        tc++;
    }
    if (tc) {
        enc->put(tc);
        if (name) {
            enc->putv(uint8_t(0x01), name);
        }
        if (version) {
            enc->putv(uint8_t(0x11), version);
        }
        if (port2) {
            enc->putv(uint8_t(0x0f), (uint32_t)port2);
        }
    }
    enc->put((uint32_t)saddr, true);
    enc->put((uint16_t)sport);
    auto data = enc->encode();
    delete enc;
    return data;
}

void Hello::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != this->magic) {
        throw Fail("Hello parse fail with invalid magic, %x expected, but %x", OP_HELLO, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    hash.set(16);
    dec.get(hash->data, hash->len);
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
                case 0x01:
                    name.reset(new Data_(vbuf, vlen, true));
                    break;
                case 0x11:
                    version.reset(new Data_(vbuf, vlen, true));
                    break;
            }
        } else if (magic == INT_TAG) {
            dec.get(nbuf, nlen, ival);
            switch ((unsigned char)nbuf[0]) {
                case 0x0F:
                    port2 = ival;
                    break;
            }
        } else {
            continue;
        }
    }
    saddr = dec.get<uint32_t, 4>(true);
    sport = dec.get<uint16_t, 2>();
}

std::string Hello::tostr() {
    std::stringstream ss;
    ss << cid;
    ss << "," << port;
    if (name) {
        ss << "," << name->data;
    }
    if (version) {
        ss << "," << version->data;
    }
    return ss.str();
}

RequestParts::RequestParts() : hash(16) {}

void RequestParts::addPart(uint32_t beg, uint32_t end) { parts.push_back(std::pair<uint32_t, uint32_t>(beg, end)); }

Data RequestParts::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_REQUESTPARTS);
    enc->put(hash->data, hash->len);
    BOOST_FOREACH (FilePart &part, parts) { enc->put(part.first); }
    BOOST_FOREACH (FilePart &part, parts) { enc->put(part.second); }
    auto data = enc->encode();
    delete enc;
    return data;
}

void RequestParts::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_REQUESTPARTS) {
        throw Fail("RequestParts parse fail with invalid magic, %x expected, but %x", OP_REQUESTPARTS, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
    size_t plen = (data->len - dec.offset) / 4;
    uint32_t pos[plen];
    for (size_t i = 0; i < plen; i++) {
        pos[i] = dec.get<uint32_t, 4>();
    }
    for (size_t i = 0; i < plen / 2; i++) {
        parts.push_back(FilePart(pos[i * 2], pos[i * 2 + 1]));
    }
}

SendingPart::SendingPart() {}

Data SendingPart::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OP_SENDINGPART);
    enc->put(hash->data, hash->len);
    enc->put((uint32_t)start);
    enc->put((uint32_t)end);
    enc->put(part->data, part->len);
    auto data = enc->encode();
    delete enc;
    return data;
}

void SendingPart::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_SENDINGPART) {
        throw Fail("SendingPart parse fail with invalid magic, %x expected, but %x", OP_SENDINGPART, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    hash.set(16);
    dec.get(hash->data, hash->len);
    start = dec.get<uint32_t, 4>();
    end = dec.get<uint32_t, 4>();
    part.reset(new Data_(end - start));
    dec.get(part->data, part->len);
}

MagicHash::MagicHash(uint8_t magic) { this->magic = magic; }
Data MagicHash::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)magic);
    enc->put(hash->data, hash->len);
    auto data = enc->encode();
    delete enc;
    return data;
}
void MagicHash::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != this->magic) {
        throw Fail("MagicHash parse fail with invalid magic, %x expected, but %x", this->magic, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
}

HashsetAnswer::HashsetAnswer() {}
Data HashsetAnswer::encode() { return Data(); }
void HashsetAnswer::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OP_HASHSETANSWER2) {
        throw Fail("HashsetAnswer parse fail with invalid magic, %x expected, but %x", OP_HASHSETANSWER, xmagic);
    }
    if (magic == OP_PACKEDPROT) {
        dec.inflate();
    }
    dec.get(hash->data, hash->len);
    uint16_t pc = dec.get<uint16_t, 2>();
    for (size_t i = 0; i < pc; i++) {
        Hash h(16);
        dec.get(h->data, h->len);
        parts.push_back(h);
    }
}

//////////end encoding//////////
}
}
