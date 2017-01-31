//
//  fs.cpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#include "fs.hpp"
#include <sqlite3.h>
#include <boost/foreach.hpp>
#include <fstream>

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

Hash BuildHash(Data &data) { return Hash(data->data, data->len); }

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

std::vector<uint8_t> SortedPart::parsePartStatus(size_t plen) {
    std::vector<uint8_t> parts;
    size_t len = size() / 2;
    uint8_t part = 0;
    long pidx = 0;
    int dc = 0;
    for (size_t i = 0; i < len; i++) {
        long offset = at(2 * i) / plen;
        if (at(2 * i) % plen > 0) {
            offset++;
        }
        dc += offset - pidx;
        while (dc > 8) {
            parts.push_back(part);
            dc -= 8;
            part = 0;
        }
        pidx = offset;
        //
        long ac = (at(2 * i + 1) - at(2 * i) + 1) / plen;
        for (long j = 0; j < ac; j++) {
            part = (1 << (7 - dc)) | part;
            dc++;
            if (dc < 8) {
                continue;
            }
            parts.push_back(part);
            dc = 0;
            part = 0;
        }
        pidx = offset + ac;
    }
    if (dc > 0) {
        parts.push_back(part);
    }
    return parts;
}

FileConf_::FileConf_(size_t size) : parts(size) {}

void FileConf_::save(const char *path) {
    Encoding enc;
    enc.put((uint8_t)0x10).put((uint16_t)filename->len).put(filename);
    enc.put((uint8_t)0x20).put((uint64_t)size);
    if (emd4.get()) {
        enc.put((uint8_t)0x30).put((uint16_t)emd4->len).put(emd4);
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
                filename = BuildData(dec.get<uint16_t, 2>(), true);
                dec.get(filename->data, filename->len);
                break;
            }
            case 0x20: {
                size = dec.get<uint64_t, 8>();
                break;
            }
            case 0x30: {
                emd4 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(emd4->data, emd4->len);
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
        if (!file.is_open()) {
            throw LFail(strlen(path) + 64, "FileConf_ read hash open path(%s) fail", path);
        }
        auto fname = boost::filesystem::path(path).filename();
        filename = BuildData(fname.c_str(), fname.size());
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
            emd4.set(MD4_DIGEST_LENGTH);
            MD4_Final((unsigned char *)emd4->data, &fmd4);
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
        throw LFail(strlen(path) + 64, "FileConf_ read hash path(%s) fail", path);
    }
}

std::vector<uint8_t> FileConf_::parsePartStatus(size_t plen) { return parts.parsePartStatus(plen); }

FileConf BuildFileConf(size_t size) { return FileConf(new FileConf_(size)); }

FileConf BuildFileConf(const char *path, bool bmd4, bool bmd5, bool bsha1) {
    auto fc = FileConf(new FileConf_());
    fc->readhash(path, bmd4, bmd5, bsha1);
    return fc;
}

File::File(boost::filesystem::path spath, size_t size) : fc(size) {
    this->spath = spath;
    this->tpath = boost::filesystem::path(spath.string() + ".xdm");
    this->cpath = boost::filesystem::path(spath.string() + ".xcm");
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

std::vector<uint8_t> File::parsePartStatus(size_t plen) { return fc.parsePartStatus(plen); }

bool File::valid() {}

FData BuildFData(const char *spath) {
    auto fc = BuildFileConf(spath, true, true, true);
    auto fd = FData(new FData_());
    fd->sha1 = fc->sha1;
    fd->md5 = fc->md5;
    fd->emd4 = fc->emd4;
    fd->filename = fc->filename;
    fd->size = fc->size;
    auto fpath = boost::filesystem::path(spath);
    if (fpath.has_extension()) {
        auto ext = fpath.extension();
        fd->format = BuildData(ext.c_str(), ext.size());
    }
    fd->location = BuildData(spath, strlen(spath));
    fd->status = FDSS_SHARING;
    return fd;
}

FDataDb_::FDataDb_() : butils::tools::SQLite_(FDSD_VER) {}

void FDataDb_::init(const char *spath) { SQLite_::init(spath, FS_VER_SQL()); }

int FDataDb_::count(int status) { return intv("select count(*) from ex_file where status=%d", status); }

std::vector<FData> FDataDb_::list(int status, int skip, int limit) {
    std::vector<FData> fs;
    auto stmt = prepare(
        "select tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,status "
        " from ex_file where status=%d limit %d,%d",
        status, skip, limit);
    while (stmt->step()) {
        int idx = 0;
        auto file = FData(new FData_);
        file->tid = stmt->intv(idx++);
        auto sha1 = stmt->blobv(idx++);
        file->sha1 = BuildHash(sha1);
        auto md5 = stmt->blobv(idx++);
        ;
        file->md5 = BuildHash(md5);
        auto emd4 = stmt->blobv(idx++);
        file->emd4 = BuildHash(emd4);
        file->filename = stmt->stringv(idx++);
        file->size = stmt->intv(idx++);
        file->format = stmt->stringv(idx++);
        file->location = stmt->stringv(idx++);
        file->duration = stmt->floatv(idx++);
        file->bitrate = stmt->floatv(idx++);
        file->codec = stmt->stringv(idx++);
        file->authors = stmt->stringv(idx++);
        file->description = stmt->stringv(idx++);
        file->album = stmt->stringv(idx++);
        file->status = (int)stmt->intv(idx++);
        fs.push_back(file);
    }
    return fs;
}

uint64_t FDataDb_::add(FData &task) {
    auto stmt = prepare(
        "insert into ex_file ("
        "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,status"
        ") values ("
        "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?"
        ") ");
    int idx = 1;
    stmt->bind(idx++);
    if (task->sha1.get()) {
        stmt->bind(idx++, task->sha1);
    } else {
        stmt->bind(idx++);
    }
    if (task->md5.get()) {
        stmt->bind(idx++, task->md5);
    } else {
        stmt->bind(idx++);
    }
    if (task->emd4.get()) {
        stmt->bind(idx++, task->emd4);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->filename);
    stmt->bind(idx++, (sqlite3_int64)task->size);
    if (task->format.get()) {
        stmt->bind(idx++, task->format);
    } else {
        stmt->bind(idx++);
    }
    if (task->location.get()) {
        stmt->bind(idx++, task->location);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->duration);
    stmt->bind(idx++, task->bitrate);
    if (task->codec.get()) {
        stmt->bind(idx++, task->codec);
    } else {
        stmt->bind(idx++);
    }
    if (task->authors.get()) {
        stmt->bind(idx++, task->authors);
    } else {
        stmt->bind(idx++);
    }
    if (task->description.get()) {
        stmt->bind(idx++, task->description);
    } else {
        stmt->bind(idx++);
    }
    if (task->album.get()) {
        stmt->bind(idx++, task->album);
    } else {
        stmt->bind(idx++);
    }
    stmt->bind(idx++, task->status);
    stmt->step();
    stmt->finalize();
    return SQLite_::intv("select last_insert_rowid()");
}

void FDataDb_::remove(uint64_t tid) { SQLite_::exec("delete from ex_file where tid=%d", tid); }

FTaskDb_::FTaskDb_() : butils::tools::SQLite_(FTSD_VER) {}

void FTaskDb_::init(const char *spath) { SQLite_::init(spath, TS_VER_SQL()); }

int FTaskDb_::count(int status) { return intv("select count(*) from ex_task where status=%d", status); }

std::vector<FTask> FTaskDb_::list(int status, int skip, int limit) {
    std::vector<FTask> ts;
    auto stmt =
        prepare("select tid,filename,location,size,done,format,used,status from ex_task where status=%d limit %d,%d",
                status, skip, limit);
    while (stmt->step()) {
        int idx = 0;
        auto task = FTask(new FTask_);
        task->tid = stmt->intv(idx++);
        task->filename = stmt->stringv(idx++);
        task->location = stmt->stringv(idx++);
        task->size = stmt->intv(idx++);
        task->done = stmt->intv(idx++);
        task->format = stmt->stringv(idx++);
        task->used = stmt->intv(idx++);
        task->status = (int)stmt->intv(idx++);
        ts.push_back(task);
    }
    return ts;
}

uint64_t FTaskDb_::add(FTask &task) {
    SQLite_::exec(
        "insert into ex_task (tid,filename,location,size,done,format,used,status) values "
        "(null,'%s','%s',%lu,%lu,'%s',%lu,%d)",
        task->filename->data, task->location->data, task->size, task->done, task->format->data, task->used,
        task->status);
    return SQLite_::intv("select last_insert_rowid()");
}

void FTaskDb_::remove(uint64_t tid) { SQLite_::exec("delete from ex_task where tid=%d", tid); }
}
}
