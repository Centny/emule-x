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
        sprintf(buf + i * 2, "%02x", (unsigned char)hash[i]);
    }
    return std::string(buf);
}

Hash BuildHash(size_t len) { return Hash(len); }

Hash BuildHash(const char *buf, size_t len) { return Hash(buf, len); }

Hash BuildHash(Data &data) { return Hash(data->data, data->len); }

Hash FromHex(const char *hex) {
    size_t len = strlen(hex);
    if (len < 2 || len % 2) {
        return Hash();
    }
    auto bys = BuildHash(len / 2);
    for (unsigned int i = 0; i < len; i += 2) {
        bys->data[i / 2] = hex2int(hex[i]) * 16 + hex2int(hex[i + 1]);
    }
    return bys;
}

SortedPart::SortedPart(uint64_t total) { this->total = total; }

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
        if (at(2 * i + 2) - at(2 * i + 1) > 1) {
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

bool SortedPart::exists(uint64_t av, uint64_t bv) {
    size_t pl = size() / 2;
    for (size_t i = 0; i < pl; i++) {
        if (bv <= at(2 * i + 1)) {
            if (av >= at(2 * i)) {
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

std::vector<Part> SortedPart::split(uint64_t max) {
    std::vector<Part> ps;
    size_t slen = total / max;
    for (size_t i = 0; i < slen; i++) {
        if (exists(i * max, i * max + max - 1)) {
            continue;
        } else {
            ps.push_back(Part(i * max, i * max + max));
        }
    }
    if (slen * max < total - 1 && !exists(slen * max, total - 1)) {
        ps.push_back(Part(slen * max, total));
    }
    return ps;
}

FData BuildFData(const char *spath) {
    auto fc = BuildFileConf(spath, ALL_HASH);
    auto fd = FData(new FData_());
    fd->sha1 = fc->fd->sha1;
    fd->md5 = fc->fd->md5;
    fd->emd4 = fc->fd->emd4;
    fd->filename = fc->fd->filename;
    fd->size = fc->fd->size;
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

std::vector<FData> parseFDataStmt(STMT stmt) {
    std::vector<FData> fs;
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

std::vector<FData> FDataDb_::list(int status, int skip, int limit) {
    auto stmt = prepare(
        "select tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,status "
        " from ex_file where status=%d limit %d,%d",
        status, skip, limit);
    return parseFDataStmt(stmt);
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

FData FDataDb_::find(Hash &hash, int type) {
    STMT stmt;
    switch (type) {
        case 0:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where sha=?");
            break;
        case 1:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where md5=?");
            break;
        case 2:
            stmt = prepare(
                "select "
                "tid,sha,md5,emd4,filename,size,format,location,duration,bitrate,codec,authors,description,album,"
                "status "
                " from ex_file where emd4=?");
            break;
        default:
            throw Fail("FDataDb_ find unknow type(%d)", type);
            break;
    }
    stmt->bind(1, hash);
    auto fs = parseFDataStmt(stmt);
    if (fs.size()) {
        return fs[0];
    } else {
        return FData();
    }
}

void FDataDb_::updateFilename(uint64_t tid, Data filename) {
    auto stmt = prepare("update ex_file set filename=? where tid=?");
    stmt->bind(1, filename);
    stmt->bind(2, (sqlite3_int64)tid);
    stmt->step();
}

EmuleX_::EmuleX_() : butils::tools::SQLite_(EX_DB_VER) {}

void EmuleX_::init(const char *spath) { SQLite_::init(spath, EX_VER_SQL()); }

int EmuleX_::countTask(int status) { return intv("select count(*) from ex_task where status=%d", status); }

std::vector<FTask> EmuleX_::listTask(int status, int skip, int limit) {
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

uint64_t EmuleX_::addTask(FTask &task) {
    auto format = "";
    if (task->format.get()) {
        format = task->format->data;
    }
    SQLite_::exec(
        "insert into ex_task (tid,filename,location,size,done,format,used,status) values "
        "(null,'%s','%s',%lu,%lu,'%s',%lu,%d)",
        task->filename->data, task->location->data, task->size, task->done, format, task->used, task->status);
    auto tid = SQLite_::intv("select last_insert_rowid()");
    task->tid = tid;
    return tid;
}

FTask EmuleX_::addTask(boost::filesystem::path dir, FData &file) {
    auto task = FTask(new FTask_);
    task->filename = file->filename;
    task->location = BuildData(dir.c_str(), dir.size());
    task->size = file->size;
    task->format = file->format;
    task->status = FTSS_RUNNING;
    addTask(task);
    return task;
}

void EmuleX_::removeTask(uint64_t tid) { SQLite_::exec("delete from ex_task where tid=%d", tid); }

FileConf_::FileConf_(size_t size) : parts(size) { fd = FData(new FData_); }

void FileConf_::save(const char *path) {
    Encoding enc;
    enc.put((uint8_t)0x10).put((uint16_t)fd->filename->len).put(fd->filename);
    enc.put((uint8_t)0x20).put((uint64_t)fd->size);
    if (fd->emd4.get()) {
        enc.put((uint8_t)0x30).put((uint16_t)fd->emd4->len).put(fd->emd4);
    }
    if (ed2k.size()) {
        enc.put((uint8_t)0x40).put((uint16_t)ed2k.size());
        BOOST_FOREACH (Hash &h, ed2k) {
            enc.put((uint16_t)h->len);
            enc.put(h);
        }
    }
    if (fd->md5.get()) {
        enc.put((uint8_t)0x50).put((uint16_t)fd->md5->len).put(fd->md5);
    }
    if (fd->sha1.get()) {
        enc.put((uint8_t)0x60).put((uint16_t)fd->sha1->len).put(fd->sha1);
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
    if (!file.is_open()) {
        throw Fail("FileConf_ open path(%s) fail", path);
    }
    Data data = BuildData(file.tellg());
    file.seekg(0);
    file.read(data->data, data->len);
    file.close();
    Decoding dec(data);
    while (dec.offset < data->len) {
        switch (dec.get<uint8_t, 1>()) {
            case 0x10: {
                fd->filename = BuildData(dec.get<uint16_t, 2>(), true);
                dec.get(fd->filename->data, fd->filename->len);
                break;
            }
            case 0x20: {
                fd->size = dec.get<uint64_t, 8>();
                parts.total = fd->size;
                break;
            }
            case 0x30: {
                fd->emd4 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(fd->emd4->data, fd->emd4->len);
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
                fd->md5 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(fd->md5->data, fd->md5->len);
                break;
            }
            case 0x60: {
                fd->sha1 = BuildHash(dec.get<uint16_t, 2>());
                dec.get(fd->sha1->data, fd->sha1->len);
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

int FileConf_::readhash(const char *path, HashType type) {
    std::fstream file;
    try {
        file.open(path, std::fstream::in | std::fstream::ate);
        if (!file.is_open()) {
            throw LFail(strlen(path) + 64, "FileConf_ read hash open path(%s) fail", path);
        }
        auto fname = boost::filesystem::path(path).filename();
        fd->filename = BuildData(fname.c_str(), fname.size());
        readhash(&file, type);
        file.close();
    } catch (...) {
        file.close();
        throw LFail(strlen(path) + 64, "FileConf_ read hash path(%s) fail", path);
    }
}
int FileConf_::readhash(std::fstream *file, HashType type) {
    MD4_CTX fmd4;
    if (HASH_IS(type, EMD4)) {
        MD4_Init(&fmd4);
    }
    MD5_CTX fmd5;
    if (HASH_IS(type, MD5)) {
        MD5_Init(&fmd5);
    }
    SHA_CTX fsha;
    if (HASH_IS(type, SHA1)) {
        SHA1_Init(&fsha);
    }
    fd->size = file->tellg();
    parts.total = fd->size;
    file->seekg(0);
    file->seekp(0);
    char buf[9728];
    size_t readed = 0;
    unsigned char digest[MD4_DIGEST_LENGTH];
    while (!file->eof()) {
        MD4_CTX pmd4;
        if (HASH_IS(type, EMD4)) {
            MD4_Init(&pmd4);
        }
        for (int i = 0; i < 1000 && !file->eof(); i++) {
            file->read(buf, 9728);
            readed = file->gcount();
            if (HASH_IS(type, EMD4)) {
                MD4_Update(&pmd4, buf, readed);
            }
            if (HASH_IS(type, MD5)) {
                MD5_Update(&fmd5, buf, readed);
            }
            if (HASH_IS(type, SHA1)) {
                SHA1_Update(&fsha, buf, readed);
            }
            printf("-->%ld\n", readed);
        }
        if (HASH_IS(type, EMD4)) {
            MD4_Final(digest, &pmd4);
            ed2k.push_back(BuildHash((const char *)digest, MD4_DIGEST_LENGTH));
            MD4_Update(&fmd4, digest, MD4_DIGEST_LENGTH);
        }
    }
    if (HASH_IS(type, EMD4)) {
        fd->emd4.set(MD4_DIGEST_LENGTH);
        MD4_Final((unsigned char *)fd->emd4->data, &fmd4);
        if (ed2k.size() == 1) {
            fd->emd4 = ed2k[0];
        }
    }
    if (HASH_IS(type, MD5)) {
        fd->md5.set(MD5_DIGEST_LENGTH);
        MD5_Final((unsigned char *)fd->md5->data, &fmd5);
    }
    if (HASH_IS(type, SHA1)) {
        fd->sha1.set(SHA_DIGEST_LENGTH);
        SHA1_Final((unsigned char *)fd->sha1->data, &fsha);
    }
    return 0;
}

std::vector<uint8_t> FileConf_::parsePartStatus(size_t plen) { return parts.parsePartStatus(plen); }

std::vector<Part> FileConf_::split(uint64_t max) { return parts.split(max); }

FileConf BuildFileConf(size_t size) { return FileConf(new FileConf_(size)); }

FileConf BuildFileConf(const char *path, HashType type) {
    auto fc = FileConf(new FileConf_(0));
    fc->readhash(path, type);
    return fc;
}

File_::File_(boost::filesystem::path dir, FData &file) {
    this->fc = FileConf(new FileConf_(file->size));
    this->fc->fd = file;
    this->spath = dir.append(file->filename->data);
    this->tpath = boost::filesystem::path(spath.string() + ".xdm");
    this->cpath = boost::filesystem::path(spath.string() + ".xcm");
    this->fc->save(cpath.c_str());
    this->fs = new std::fstream();
    this->fs->open(tpath.c_str(), std::fstream::out | std::fstream::binary);
    if (!this->fs->is_open()) {
        throw Fail("File_ open file(%s) fail", tpath.c_str());
    }
    if (this->fs->tellg() < 1) {
        fs->seekp(this->fc->fd->size - 1);
        fs->write("\0", 1);
    }
}

File_::File_(boost::filesystem::path dir, Data &filename) {
    this->fc = FileConf(new FileConf_(0));
    this->spath = dir.append(filename->data);
    this->tpath = boost::filesystem::path(spath.string() + ".xdm");
    this->cpath = boost::filesystem::path(spath.string() + ".xcm");
    this->fc->read(this->cpath.c_str());
    this->fs = new std::fstream();
    this->fs->open(tpath.c_str(), std::fstream::out | std::fstream::binary);
    if (!this->fs->is_open()) {
        throw Fail("File_ open file(%s) fail", tpath.c_str());
    }
    if (this->fs->tellg() < 1) {
        fs->seekp(this->fc->fd->size - 1);
        fs->write("\0", 1);
    }
}

File_::~File_() { close(); }

bool File_::exists(size_t av, size_t bv) { return fc->exists(av, bv); }

bool File_::write(size_t offset, Data data) { return write(offset, data->data, data->len); }

bool File_::write(size_t offset, const char *data, size_t len) {
    fs->seekp(offset);
    fs->write(data, len);
    bool done = fc->add(offset, offset + len - 1);
    fc->save(cpath.c_str());
    return done;
}

void File_::read(size_t offset, Data data) {
    fs->seekg(offset);
    fs->read(data->data, data->len);
}

bool File_::isdone() { return fc->isdone(); }

std::vector<uint8_t> File_::parsePartStatus(size_t plen) { return fc->parsePartStatus(plen); }

std::vector<Part> File_::split(uint64_t max) { return fc->split(max); }

bool File_::valid(HashType type) {
    fs->flush();
    auto cfc = FileConf(new FileConf_(0));
    cfc->readhash(tpath.c_str(), type);
    if (HASH_IS(type, EMD4)) {
        fc->fd->emd4->print();
        cfc->fd->emd4->print();
        return cfc->fd->emd4->cmp(fc->fd->emd4);
    }
    if (HASH_IS(type, MD5)) {
        return cfc->fd->md5->cmp(fc->fd->md5);
    }
    if (HASH_IS(type, SHA1)) {
        return cfc->fd->sha1->cmp(fc->fd->sha1);
    }
    return false;
}

void File_::close() {
    if (fs) {
        fs->close();
        fs = 0;
    }
}

void File_::done() {
    close();
    boost::filesystem::rename(tpath, spath);
    boost::filesystem::remove_all(cpath);
}

FileManager_::FileManager_(const char *emulex, const char *fdb) {
    ts = EmuleX(new EmuleX_);
    fs = FDataDb(new FDataDb_);
    ts->init(emulex);
    fs->init(fdb);
}

File FileManager_::findOpenedF(Hash &hash) { return opened.at(hash); }

File FileManager_::open(boost::filesystem::path dir, FData &file) {
    auto f = File(new File_(dir, file));
    if (f->fc->fd->sha1.get()) {
        opened[f->fc->fd->sha1] = f;
    }
    if (f->fc->fd->md5.get()) {
        opened[f->fc->fd->md5] = f;
    }
    if (f->fc->fd->emd4.get()) {
        opened[f->fc->fd->emd4] = f;
    }
    return f;
}

File FileManager_::open(boost::filesystem::path dir, Data &filename) {
    auto f = File(new File_(dir, filename));
    if (f->fc->fd->sha1.get()) {
        opened[f->fc->fd->sha1] = f;
    }
    if (f->fc->fd->md5.get()) {
        opened[f->fc->fd->md5] = f;
    }
    if (f->fc->fd->emd4.get()) {
        opened[f->fc->fd->emd4] = f;
    }
    return f;
}

void FileManager_::done(Hash &hash) {
    auto file = findOpenedF(hash);
    file->done();
    opened.erase(hash);
}
//
}
}
