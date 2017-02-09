//
//  kadx_protocol.cpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#include "kadx_protocol.hpp"

namespace emulex {
namespace protocol {
namespace pkadx {
ModH BuildMod() {
    auto mod = ModH(new M1L4());
    M1L4 *m = (M1L4 *)mod.get();
    m->magic[0] = OPX_KADX_HEAD;
    m->magic[1] = OPX_KADX_HEAD_C;
    m->big = false;
    return mod;
}

Login_::Login_() : hash(20) {
    addr = 0;
    port = 0;
    version = 0;
    flags = 0;
}

Login_::Login_(Hash &hash, uint32_t version, Data &name, uint32_t addr, uint16_t port, uint32_t flags) {
    this->hash = hash;
    this->name = name;
    this->addr = addr;
    this->port = port;
    this->version = version;
    this->flags = flags;
}

Data Login_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(hash->data, hash->len);
    enc->put(version);
    enc->put((uint8_t)name->len);
    enc->put(name);
    enc->put(addr);
    enc->put(port);
    enc->put(flags);
    auto data = enc->encode();
    delete enc;
    return data;
}

void Login_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_LOGIN && xmagic != OPX_LOGIN_BACK) {
        throw Fail("Login parse fail with invalid magic, %x,%x expected, but %x", OPX_LOGIN, OPX_LOGIN_BACK, xmagic);
    }
    dec.get(hash->data, hash->len);
    version = dec.get<uint32_t, 4>();
    name.reset(new Data_(dec.get<uint8_t, 1>(), true));
    dec.get(name->data, name->len);
    addr = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
    flags = dec.get<uint32_t, 4>();
}

size_t Login_::show(char *buf) {
    char tbuf[512];
    size_t tlen = 0;
    tlen += sprintf(tbuf + tlen, "\n Hash:%s", hash.tostring().c_str());
    tlen += sprintf(tbuf + tlen, "\n Version:%0x", version);
    tlen += sprintf(tbuf + tlen, "\n Name:%s", name->data);
    tlen += sprintf(tbuf + tlen, "\n Addr:%d", addr);
    tlen += sprintf(tbuf + tlen, "\n Port:%d", port);
    tlen += sprintf(tbuf + tlen, "\n Flags:%0x", flags);
    tbuf[tlen] = 0;
    if (buf == 0) {
        printf("%s\n", tbuf);
    }
    return tlen;
}

bool TaskComparer::operator()(const Task &first, const Task &second) const {
    if (first->uid.get() && second->uid.get()) {
        if (first->uid->cmp(second->uid.get())) {
            return DataComparer()(first->uid, second->uid);
        } else {
            return first->tid < second->tid;
        }
    }
    return false;
}

Search_::Search_() {}
Search_::Search_(Hash &uid, uint16_t tid, Data &key, uint32_t flags) {
    this->uid = uid;
    this->tid = tid;
    this->key = key;
    this->flags = flags;
}
Data Search_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(uid);
    enc->put(tid);
    enc->put((uint16_t)key->len);
    enc->put(key);
    enc->put(flags);
    auto data = enc->encode();
    delete enc;
    return data;
}
void Search_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_SEARCH) {
        throw Fail("Search parse fail with invalid magic, %x expected, but %x", OPX_SEARCH, xmagic);
    }
    uid.set(HASH_UID_L);
    dec.get(uid->data, uid->len);
    tid = dec.get<uint16_t, 2>();
    key.reset(new Data_(dec.get<uint16_t, 2>(), true));
    dec.get(key->data, key->len);
    flags = dec.get<uint32_t, 4>();
}

SearchBack_::SearchBack_() {}
SearchBack_::SearchBack_(Hash &uid, uint16_t tid, uint32_t flags) {
    this->uid = uid;
    this->tid = tid;
    this->flags = flags;
}
std::vector<Data> SearchBack_::encode(uint8_t magic) {
    size_t len = fs.size();
    std::vector<Data> ts;
    int i = 0;
    while (true) {
        uint8_t fc = 0;
        Encoding fenc;
        while (i < len) {
            fs[i]->encode(fenc);
            i++;
            fc++;
            if (fenc.size() * 70 >= 100000) {
                break;
            }
            if (i + 1 < len && (fenc.size() + fs[i + 1]->dsize()) * 70 >= 100000) {
                break;
            }
        }
        Data fdata;
        Encoding enc;
        if (fenc.size() > 1000) {
            fdata = fenc.deflate();
            enc.put((uint8_t)OPX_SEARCH_BC);
        } else {
            fdata = fenc.encode();
            enc.put((uint8_t)OPX_SEARCH_BACK);
        }
        enc.put(magic);
        enc.put(uid);
        enc.put(tid);
        enc.put(flags);
        enc.put(fc);
        enc.put(fdata);
        auto data = enc.encode();
        ts.push_back(data);
    }
    return ts;
}
void SearchBack_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_SEARCH_BACK && xmagic != OPX_SEARCH_BC) {
        throw Fail("SearchBack parse fail with invalid magic, %x,%x expected, but %x", OPX_SEARCH_BACK, OPX_SEARCH_BC,
                   xmagic);
    }
    uid.set(HASH_UID_L);
    dec.get(uid->data, uid->len);
    tid = dec.get<uint16_t, 2>();
    flags = dec.get<uint32_t, 4>();
    auto fc = dec.get<uint8_t, 1>();
    if (xmagic == OPX_SEARCH_BC) {
        dec.inflate();
    }
    while (fc) {
        FData fd = FData(new FData_);
        fd->parse(dec);
        fs.push_back(fd);
        fc -= 1;
    }
}

SearchSource_::SearchSource_() {}

Data SearchSource_::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OPX_SSRC);
    enc->put(uid);
    enc->put(tid);
    enc->put((uint8_t)hash.type);
    enc->put(hash->data, hash->len);
    enc->put(size);
    auto data = enc->encode();
    delete enc;
    return data;
}

void SearchSource_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_SSRC) {
        throw Fail("SearchSource parse fail with invalid magic, %x expected, but %x", OPX_SSRC, xmagic);
    }
    uid.set(HASH_UID_L);
    dec.get(uid->data, uid->len);
    tid = dec.get<uint16_t, 2>();
    hash.set((HashType)dec.get<uint8_t, 1>());
    dec.get(hash->data, hash->len);
    size = dec.get<uint64_t, 8>();
}

KadxAddr_::KadxAddr_(uint32_t addr, uint16_t port, uint16_t cid) : addr(addr), port(port), cid(cid) {}

FoundSource_::FoundSource_() {}

Data FoundSource_::encode() {
    auto enc = new Encoding();
    enc->put((uint8_t)OPX_FSRC);
    enc->put(uid);
    enc->put(tid);
    enc->put((uint8_t)hash.type);
    enc->put(hash->data, hash->len);
    enc->put((uint8_t)srvs.size());
    BOOST_FOREACH (const KadxAddr &srv, srvs) {
        enc->put(srv->addr);
        enc->put(srv->port);
        enc->put(srv->cid);
    }
    auto data = enc->encode();
    delete enc;
    return data;
}

void FoundSource_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_FSRC) {
        throw Fail("FoundSource parse fail with invalid magic, %x expected, but %x", OPX_FSRC, xmagic);
    }
    uid.set(HASH_UID_L);
    dec.get(uid->data, uid->len);
    tid = dec.get<uint16_t, 2>();
    hash.set((HashType)dec.get<uint8_t, 1>());
    dec.get(hash->data, hash->len);
    uint8_t count = dec.get<uint8_t, 1>();
    for (uint8_t i = 0; i < count; i++) {
        auto addr = KadxAddr(new KadxAddr_);
        addr->addr = dec.get<uint32_t, 4>();
        addr->port = dec.get<uint16_t, 2>();
        addr->cid = dec.get<uint64_t, 8>();
        srvs.push_back(addr);
    }
}

Data Callback_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(cid);
    enc->put(addr);
    enc->put(port);
    auto data = enc->encode();
    delete enc;
    return data;
}
void Callback_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_CALLBACK) {
        throw Fail("Callback parse fail with invalid magic, %x expected, but %x", OPX_CALLBACK, xmagic);
    }
    cid = dec.get<uint64_t, 8>();
    addr = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
}

Data Hole_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(cid);
    enc->put(flags);
    auto data = enc->encode();
    delete enc;
    return data;
}
void Hole_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_HOLE) {
        throw Fail("Hole parse fail with invalid magic, %x expected, but %x", OPX_HOLE, xmagic);
    }
    cid = dec.get<uint64_t, 8>();
    flags = dec.get<uint32_t, 4>();
}

Data HoleCallback_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(addr);
    enc->put(port);
    auto data = enc->encode();
    delete enc;
    return data;
}
void HoleCallback_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_HOLE_CALLBACK) {
        throw Fail("HoleCallback parse fail with invalid magic, %x expected, but %x", OPX_HOLE_CALLBACK, xmagic);
    }
    addr = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
}

Data Holing_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put(addr);
    enc->put(port);
    enc->put(type);
    auto data = enc->encode();
    delete enc;
    return data;
}
void Holing_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_HOLING) {
        throw Fail("Holing parse fail with invalid magic, %x expected, but %x", OPX_HOLING, xmagic);
    }
    addr = dec.get<uint32_t, 4>();
    port = dec.get<uint16_t, 2>();
    type = dec.get<uint8_t, 1>();
}
Data FileStatus_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put((uint8_t)hash.type);
    enc->put(hash->data, hash->len);
    auto data = enc->encode();
    delete enc;
    return data;
}
void FileStatus_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_FILE_STATUS) {
        throw Fail("FileStatus parse fail with invalid magic, %x expected, but %x", OPX_FILE_STATUS, xmagic);
    }
    hash.set((HashType)dec.get<uint8_t, 1>());
    dec.get(hash->data, hash->len);
}
    
    FilePart_::FilePart_():parts(0){
        
    }

Data FilePart_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put((uint8_t)hash.type);
    enc->put(hash->data, hash->len);
    enc->put((uint64_t)parts.total);
    enc->put((uint8_t)parts.size());
    BOOST_FOREACH (const uint64_t &v, parts) { enc->put(v); }
    auto data = enc->encode();
    delete enc;
    return data;
}
void FilePart_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_FILE_STATUS_BACK && xmagic != OPX_FILE_PART) {
        throw Fail("FilePart parse fail with invalid magic, %x,%x expected, but %x", OPX_FILE_STATUS_BACK,
                   OPX_FILE_PART, xmagic);
    }
    hash.set((HashType)dec.get<uint8_t, 1>());
    dec.get(hash->data, hash->len);
    parts.total=dec.get<uint64_t, 8>();
    uint8_t sc = dec.get<uint8_t, 1>();
    for (uint8_t i = 0; i < sc; i++) {
        parts.push_back(dec.get<uint64_t, 8>());
    }
}

Data FileProc_::encode(uint8_t magic) {
    auto enc = new Encoding();
    enc->put(magic);
    enc->put((uint8_t)hash.type);
    enc->put(hash->data, hash->len);
    enc->put(offset);
    enc->put((uint64_t)data->len);
    enc->put(data);
    auto data = enc->encode();
    delete enc;
    return data;
}

void FileProc_::parse(Data &data, uint8_t magic) {
    Decoding dec(data);
    uint8_t xmagic = dec.get<uint8_t, 1>();
    if (xmagic != OPX_FILE_PROC) {
        throw Fail("FileProc parse fail with invalid magic, %x expected, but %x", OPX_FILE_PROC, xmagic);
    }
    hash.set((HashType)dec.get<uint8_t, 1>());
    dec.get(hash->data, hash->len);
    offset = dec.get<uint64_t, 8>();
    this->data = data;
}

const char *FileProc_::cdata() { return data->data + (hash->len + 9); }
}
}
}
