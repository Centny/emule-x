//
//  kadx_protocol.hpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#ifndef kadx_protocol_hpp
#define kadx_protocol_hpp
#include <lzma.h>
#include <boost-utils/boost-utils.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include "../encoding/encoding.hpp"
#include "../fs/fs.hpp"
#include "kadx_opcodes.h"
namespace emulex {
namespace protocol {
namespace pkadx {
using namespace boost;
using namespace butils::netw;
using namespace emulex::fs;
using namespace emulex::encoding;
ModH BuildMod();
class Login_ {
   public:
    Hash hash;
    uint16_t version;
    Data name;
    uint32_t addr;
    uint16_t port;
    uint32_t flags;

   public:
    Login_();
    Login_(Hash &hash, uint32_t version, Data &name, uint32_t addr, uint16_t port, uint32_t flags = 0);
    virtual Data encode(uint8_t magic = OPX_LOGIN);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
    virtual size_t show(char *buf = 0);
};
typedef boost::shared_ptr<Login_> Login;

class Task_ {
   public:
    Hash uid;
    uint16_t tid;
};

typedef boost::shared_ptr<Task_> Task;
struct TaskComparer {
    bool operator()(const Task &first, const Task &second) const;
};
//
class Search_ : public Task_ {
   public:
    Data key;
    uint32_t flags = 0;

   public:
    Search_();
    Search_(Hash &uid, uint16_t tid, Data &key, uint32_t flags = 0);
    virtual Data encode(uint8_t magic = OPX_SEARCH);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<Search_> Search;
//
class SearchBack_ : public Task_ {
   public:
    uint32_t flags = 0;
    std::vector<FData> fs;

   public:
    SearchBack_();
    SearchBack_(Hash &uid, uint16_t tid, uint32_t flags = 0);
    virtual std::vector<Data> encode(uint8_t magic = OPX_SEARCH_BACK);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
    //        virtual size_t show(char *buf = 0);
};
typedef boost::shared_ptr<SearchBack_> SearchBack;
//
class SearchSource_ : public Task_ {
   public:
    Hash hash;
    uint64_t size;

   public:
    SearchSource_();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<SearchSource_> SearchSource;
//
class KadxAddr_ {
   public:
    uint32_t addr;
    uint16_t port;
    uint64_t cid;

   public:
    KadxAddr_(uint32_t addr = 0, uint16_t port = 0, uint16_t cid = 0);
};
typedef boost::shared_ptr<KadxAddr_> KadxAddr;
//
class FoundSource_ : public Task_ {
   public:
    Hash hash;
    std::vector<KadxAddr> srvs;

   public:
    FoundSource_();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<FoundSource_> FoundSource;
//
class Callback_ {
   public:
    uint64_t cid;
    uint32_t addr;
    uint16_t port;

   public:
    virtual Data encode(uint8_t magic = OPX_CALLBACK);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<Callback_> Callback;
//
class Hole_ {
   public:
    uint64_t cid;
    uint32_t flags;

   public:
    virtual Data encode(uint8_t magic = OPX_HOLE);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<Hole_> Hole;
//
class HoleCallback_ {
   public:
    uint32_t addr;
    uint16_t port;

   public:
    virtual Data encode(uint8_t magic = OPX_HOLE_CALLBACK);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<HoleCallback_> HoleCallback;
//
class HolePending_ {
   public:
    Hash uid;

   public:
    virtual Data encode(uint8_t magic = OPX_HOLE_PENDING);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<HolePending_> HolePending;
//
class Holing_ {
   public:
    uint32_t addr;
    uint16_t port;
    uint8_t type;

   public:
    virtual Data encode(uint8_t magic = OPX_HOLING);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<Holing_> Holing;
//
class FileStatus_ {
   public:
    Hash hash;

   public:
    virtual Data encode(uint8_t magic = OPX_FILE_STATUS);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<FileStatus_> FileStatus;
//
class FilePart_ {
   public:
    Hash hash;
    SortedPart parts;

   public:
    FilePart_();
    virtual Data encode(uint8_t magic = OPX_FILE_PART);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
};
typedef boost::shared_ptr<FilePart_> FilePart;
//
class FileProc_ {
   public:
    Hash hash;
    uint64_t offset;
    Data data;

   public:
    virtual Data encode(uint8_t magic = OPX_FILE_PROC);
    virtual void parse(Data &data, uint8_t magic = OPX_KADX_HEAD);
    virtual const char *cdata();
};
typedef boost::shared_ptr<FileProc_> FileProc;
}
}
}
#endif /* kadx_protocol_hpp */
