//
//  fs.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef fs_hpp
#define fs_hpp

#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <boost-utils/boost-utils.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <tools/sqlite.hpp>
#include <vector>
#include "../encoding/encoding.hpp"
#include "iconv.h"

namespace emulex {
namespace fs {
using namespace butils::netw;
using namespace butils::tools;
using namespace emulex::encoding;
#define ED2K_PART_L 9728000
#define EMULEX_FSDB_VER 1
#define EMULEX_TSDB_VER 1
//
#define EMULEX_FORMAT_L 8
/*
 the ed2k hash
 */
class Hash : public Data {
   public:
    Hash();
    Hash(size_t len);
    Hash(const char *buf, size_t len);
    virtual ~Hash();
    virtual void set(size_t len);
    virtual void set(const char *buf, size_t len);
    virtual bool operator==(const Hash &h) const;
    virtual bool operator<(const Hash &h) const;
    virtual std::string tostring();
};
Hash BuildHash(size_t len = 16);
Hash BuildHash(const char *buf, size_t len = 16);

std::string hash_tos(const char *hash);

class SortedPart : public std::vector<uint64_t> {
   public:
    size_t total;

   public:
    SortedPart(size_t total);
    virtual bool add(uint64_t av, uint64_t bv);
    virtual bool exists(size_t offset, size_t len);
    virtual bool isdone();
    virtual void print();
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
};

#define write_data(file_, key_, data_)                                                        \
    file_.write(key_, 1)                                                                      \
        .write(endian::endian_buffer<endian::order::big, uint16_t, 16>(data_->len).data(), 2) \
        .write(data_->data, data_->len)
#define write_ui64(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint64_t, 64>(val_).data(), 8)
#define write_ui32(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint32_t, 32>(val_).data(), 4)
#define write_ui16(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint16_t, 16>(val_).data(), 2)
#define write_ui8(file_, key_, val_) \
    file_.write(key_, 1).write(endian::endian_buffer<endian::order::big, uint8_t, 8>(val_).data(), 1)
#define write_ui64_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint64_t, 64>(val_).data(), 8)
#define write_ui32_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint32_t, 32>(val_).data(), 4)
#define write_ui16_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint16_t, 16>(val_).data(), 2)
#define write_ui8_(file_, val_) file_.write(endian::endian_buffer<endian::order::big, uint8_t, 8>(val_).data(), 1)

class FileConf_ {
   public:
    Data name;               // 0x10
    uint64_t size;           // 0x20
    Hash md4;                // 0x30
    std::vector<Hash> ed2k;  // 0x40
    Hash md5;                // 0x50
    Hash sha1;               // 0x60
    SortedPart parts;        // 0x70
   public:
    FileConf_(size_t size = 0);
    virtual void save(const char *path);
    virtual void read(const char *path);
    virtual bool add(uint64_t av, uint64_t bv);
    virtual bool exists(size_t offset, size_t len);
    virtual bool isdone();
    virtual int readhash(const char *path, bool bmd4 = false, bool bmd5 = false, bool bsha1 = false);
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
};
typedef boost::shared_ptr<FileConf_> FileConf;
FileConf BuildFileConf(size_t size);

class File {
   public:
    FileConf_ fc;
    std::fstream *fs;
    boost::filesystem::path spath;
    boost::filesystem::path tpath;
    boost::filesystem::path cpath;

   public:
    File(boost::filesystem::path spath, size_t size);
    virtual bool exists(size_t offset, size_t len);
    virtual bool write(size_t offset, Data data);
    virtual void read(size_t offset, Data data);
    virtual bool isdone();
    virtual std::vector<uint8_t> parsePartStatus(size_t plen = ED2K_PART_L);
    virtual bool valid();
};

class FileManager {
   public:
};

//
#define FDSS_SHARING 100
#define FDSD_VER 100
std::map<int, const char *> FS_VER_SQL();
class FData_ {
   public:
    Hash sha1;
    Hash md5;
    Hash emd4;
    Data filename;
    uint64_t size = 0;
    Data format;
    Data location;
    double duration;
    double bitrate;
    Data codec;
    Data authors;
    Data description;
    int status=0;
};
typedef boost::shared_ptr<FData_> FData;

 class FDataDb_ : public SQLite_ {
   public:
    virtual void init(const char *spath);
     virtual int count(int status = FDSS_SHARING);
     virtual std::vector<FData> list(int status = FDSS_SHARING, int skip = 0, int limit = 30);
     virtual uint64_t add(FData &task);
     virtual void remove(uint64_t tid);
};
    typedef boost::shared_ptr<FDataDb_> FDataDb;
//
#define FTSS_DONE 200
#define FTSS_RUNNING 100
#define FTSD_VER 100
std::map<int, const char *> TS_VER_SQL();
class FTask_ {
   public:
    uint64_t tid = 0;
    Data filename;
    Data location;
    uint64_t size = 0;
    uint64_t done = 0;
    Data format;
    uint64_t used = 0;
    int status = 0;
};
typedef boost::shared_ptr<FTask_> FTask;
//
class FTaskDb_ : public SQLite_ {
   public:
    FTaskDb_();
    virtual void init(const char *spath);
    virtual int count(int status = FTSS_RUNNING);
    virtual std::vector<FTask> list(int status = FTSS_RUNNING, int skip = 0, int limit = 30);
    virtual uint64_t add(FTask &task);
    virtual void remove(uint64_t tid);
};
typedef boost::shared_ptr<FTaskDb_> FTaskDb;
}
}

#endif /* fs_hpp */
