//
//  fs.cpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#include "fs.hpp"
#include <boost/foreach.hpp>
#include <fstream>
#include <sqlite3.h>

namespace emulex {
namespace fs {

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
SortedPart::SortedPart(size_t total) { this->total = total; }
bool SortedPart::add(uint64_t av, uint64_t bv) {
    size_t len = size() / 2;
    size_t i = 0;
    for (; i < len; i++) {
        if ((long)(av - at(2 * i)) <= 1) {
            if (bv < at(2 * i)) {
                insert(begin() + 2 * i, bv);
                insert(begin() + 2 * i, av);
                return false;
            }
            if (bv <= at(2 * i + 1)) {
                (*this)[2 * i] = av;
                return false;
            }
            (*this)[2 * i] = av;
            (*this)[2 * i + 1] = bv;
            break;
        }
        if ((long)(av - at(2 * i + 1)) <= 1) {
            if (bv <= at(2 * i + 1)) {
                return false;
            }
            (*this)[2 * i + 1] = bv;
            break;
        }
    }
    if (i == len) {
        insert(begin() + 2 * i, bv);
        insert(begin() + 2 * i, av);
        return isdone();
    }
    for (; i < len - 1;) {
        if (at(2 * i + 1) < at(2 * i + 2)) {
            break;
        }
        if (at(2 * i + 1) >= at(2 * i + 3)) {
            erase(begin() + 2 * i + 3);
            erase(begin() + 2 * i + 2);
        } else {
            erase(begin() + 2 * i + 2);
            erase(begin() + 2 * i + 1);
        }
        len--;
    }
    return isdone();
}

bool SortedPart::exists(size_t offset, size_t len) {
    size_t pl = size() / 2;
    for (size_t i = 0; i < pl; i++) {
        if (offset + len <= at(2 * i + 1)) {
            if (offset >= at(2 * i + 1)) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

bool SortedPart::isdone() { return size() == 2 && at(0) == 0 && at(1) == total - 1; }

void SortedPart::print() {
    size_t len = size();
    for (size_t i = 0; i < len; i++) {
        printf("%llu ", at(i));
    }
    printf("\n");
}
    
std::vector<uint8_t> SortedPart::parsePartStatus(size_t plen){
    std::vector<uint8_t> parts;
    size_t len = size() / 2;
    uint8_t part=0;
    long pidx=0;
    int dc=0;
    for (size_t i = 0; i < len; i++) {
        long offset=at(2*i)/plen;
        if(at(2*i)%plen>0){
            offset++;
        }
        dc+=offset-pidx;
        while(dc>8){
            parts.push_back(part);
            dc-=8;
            part=0;
        }
        pidx=offset;
        //
        long ac=(at(2*i+1)-at(2*i)+1)/plen;
        for (long j=0; j<ac; j++) {
            part=(1<<(7-dc))|part;
            dc++;
            if(dc<8){
                continue;
            }
            parts.push_back(part);
            dc=0;
            part=0;
        }
        pidx=offset+ac;
    }
    if(dc>0){
        parts.push_back(part);
    }
    return parts;
}

FileConf_::FileConf_(size_t size) : parts(size) {}

void FileConf_::save(const char *path) {
    Encoding enc;
    enc.put((uint8_t)0x10).put((uint16_t)name->len).put(name);
    enc.put((uint8_t)0x20).put((uint64_t)size);
    if (md4.get()) {
        enc.put((uint8_t)0x30).put((uint16_t)md4->len).put(md4);
    }
    if (ed2k.size()) {
        enc.put((uint8_t)0x40).put((uint16_t)ed2k.size());
        BOOST_FOREACH (Hash &h, ed2k) {
            enc.put((uint16_t)h->len);
            enc.put(h);
        }
    }
    if (md5.get()) {
        enc.put((uint8_t)0x50).put((uint16_t)md5->len).put(md5);
    }
    if (sha1.get()) {
        enc.put((uint8_t)0x60).put((uint16_t)sha1->len).put(sha1);
    }
    if (parts.size()) {
        enc.put((uint8_t)0x70).put((uint16_t)parts.size());
        BOOST_FOREACH (uint64_t &v, parts) { enc.put(v); }
    }
    std::fstream file;
    file.open(path, std::fstream::out | std::fstream::trunc);
    file.write(enc.cbuf(), enc.size());
    file.close();
}

void FileConf_::read(const char *path) {
    std::fstream file;
    file.open(path, std::fstream::in | std::fstream::ate);
    Data data = BuildData(file.tellg());
    file.seekg(0);
    file.read(data->data, data->len);
    file.close();
    Decoding dec(data);
    while (dec.offset < data->len) {
        switch (dec.get<uint8_t, 1>()) {
            case 0x10: {
                name = BuildData(dec.get<uint16_t, 2>());
                dec.get(name->data, name->len);
                break;
            }
            case 0x20: {
                size = dec.get<uint64_t, 8>();
                break;
            }
            case 0x30: {
                md4 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(md4->data, md4->len);
                break;
            }
            case 0x40: {
                ed2k.clear();
                uint16_t pc = dec.get<uint16_t, 2>();
                for (size_t i = 0; i < pc; i++) {
                    Hash h = BuildHash(dec.get<uint16_t, 2>());
                    dec.get(h->data, h->len);
                    ed2k.push_back(h);
                }
                break;
            }
            case 0x50: {
                md5 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(md5->data, md5->len);
                break;
            }
            case 0x60: {
                sha1 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(sha1->data, sha1->len);
                break;
            }
            case 0x70: {
                parts.clear();
                uint16_t pc = dec.get<uint16_t, 2>();
                for (size_t i = 0; i < pc; i++) {
                    parts.push_back(dec.get<uint64_t, 8>());
                }
                break;
            }
        }
    }
}

bool FileConf_::add(uint64_t av, uint64_t bv) { return parts.add(av, bv); }

bool FileConf_::exists(size_t offset, size_t len) { return parts.exists(offset, len); }

bool FileConf_::isdone() { return parts.isdone(); }

int FileConf_::readhash(const char *path, bool bmd4, bool bmd5, bool bsha1) {
    std::fstream file;
    try {
        MD4_CTX fmd4;
        if (bmd4) {
            MD4_Init(&fmd4);
        }
        MD5_CTX fmd5;
        if (bmd5) {
            MD5_Init(&fmd5);
        }
        SHA_CTX fsha;
        if (bsha1) {
            SHA1_Init(&fsha);
        }
        file.open(path, std::fstream::in | std::fstream::ate);
        auto fname=boost::filesystem::path(path).filename();
        name = BuildData(fname.c_str(), fname.size());
        size = file.tellg();
        file.seekg(0);
        char buf[9728];
        size_t readed = 0;
        unsigned char digest[MD4_DIGEST_LENGTH];
        while (!file.eof()) {
            MD4_CTX pmd4;
            if (bmd4) {
                MD4_Init(&pmd4);
            }
            for (int i = 0; i < 1000 && !file.eof(); i++) {
                file.read(buf, 9728);
                readed = file.gcount();
                if (bmd4) {
                    MD4_Update(&pmd4, buf, readed);
                }
                if (bmd5) {
                    MD5_Update(&fmd5, buf, readed);
                }
                if (bsha1) {
                    SHA1_Update(&fsha, buf, readed);
                }
            }
            if (bmd4) {
                MD4_Final(digest, &pmd4);
                ed2k.push_back(BuildHash((const char *)digest, MD4_DIGEST_LENGTH));
                MD4_Update(&fmd4, digest, MD4_DIGEST_LENGTH);
            }
        }
        if (bmd4) {
            md4.set(MD4_DIGEST_LENGTH);
            MD4_Final((unsigned char *)md4->data, &fmd4);
        }
        if (bmd5) {
            md5.set(MD5_DIGEST_LENGTH);
            MD5_Final((unsigned char *)md5->data, &fmd5);
        }
        if (bsha1) {
            sha1.set(SHA_DIGEST_LENGTH);
            SHA1_Final((unsigned char *)sha1->data, &fsha);
        }
        file.close();
        return 0;
    } catch (...) {
        file.close();
        return -1;
    }
}
    
std::vector<uint8_t> FileConf_::parsePartStatus(size_t plen){
    return parts.parsePartStatus(plen);
}

FileConf BuildFileConf(size_t size) { return FileConf(new FileConf_(size)); }

    File::File(boost::filesystem::path spath,size_t size):fc(size){
        this->spath=spath;
        this->tpath=boost::filesystem::path(spath.string()+".xdm");
        this->cpath=boost::filesystem::path(spath.string()+".xcm");
    }
bool File::exists(size_t offset, size_t len) { return fc.exists(offset, len); }

bool File::write(size_t offset, Data data) {
    fs->seekp(offset);
    fs->write(data->data, data->len);
    bool done = fc.add(offset, offset + data->len);
    fc.save(cpath.c_str());
    return done;
}

void File::read(size_t offset, Data data) {
    fs->seekg(offset);
    fs->read(data->data, data->len);
}

bool File::isdone() { return fc.isdone(); }

std::vector<uint8_t> File::parsePartStatus(size_t plen){
    return fc.parsePartStatus(plen);
}
    
bool File::valid() {}
}

    
}
