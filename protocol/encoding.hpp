//
//  encoding.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef encoding_hpp
#define encoding_hpp
#include <stdio.h>
#include <boost-utils/boost-utils.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/endian/buffers.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>

namespace emulex {
namespace protocol {
using namespace boost;
using namespace butils::netw;
//
#include "opcodes.h"
//    const uint8_t PROTOCOL_V0 = 0xE3;
// const uint8_t PROTOCOL_V1 = 0xD4;
//
// const uint8_t OP_LOGINREQUEST = 0x01;
// const uint8_t OP_IDCHANGE = 0x40;
// const uint8_t OP_SERVERMESSAGE = 0x38;
// const uint8_t OP_SERVERSTATUS = 0x34;
// const uint8_t OP_OFFERFILES = 0x15;
// const uint8_t OP_LIST_SERVER = 0x14;
// const uint8_t OP_SERVERLIST = 0x32;
// const uint8_t OP_SERVERIDENT = 0x41;
// const uint8_t OP_SEARCHREQUEST = 0x16;

const uint8_t STR_TAG = 0x2;
const uint8_t INT_TAG = 0x3;
const uint8_t FLOAT_TAG = 0x4;
//
#define S_USR_NAME_L 128
#define S_DESC_L 256

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
//
ModH BuildMod();
/*
 the ed2k base encoding for int/string/tag
 */
class Encoding {
   public:
    Encoding();
    virtual void reset();
    virtual size_t size();
    boost::asio::streambuf &buf();
    virtual Data encode();
    Encoding &put(Data &data);
    Encoding &put(const char *val, size_t len);
    Encoding &put(uint8_t val, bool big = false);
    Encoding &put(uint16_t val, bool big = false);
    Encoding &put(uint32_t val, bool big = false);
    Encoding &put(uint64_t val, bool big = false);
    //    Encoding& put(float val);
    Encoding &putv(const char *name, const char *val);
    Encoding &putv(Data &name, Data &val);
    Encoding &putv(uint8_t name, const char *val);
    Encoding &putv(uint8_t name, Data &val);
    Encoding &putv(const char *name, Data &val);
    Encoding &putv(const char *name, uint32_t val);
    Encoding &putv(uint8_t name, uint32_t val);
    //  Encoding &put(const char *name, float val);
    //  Encoding &put(uint8_t name, float val);
    Encoding &putv(const char *name, uint64_t val);
    Encoding &putv(uint8_t name, uint64_t val);
    virtual void print(char *buf = 0);

   protected:
    boost::asio::streambuf buf_;
};

/*
 the ed2k login frame.
 */
class Decoding {
   public:
    Decoding(Data &data);
    virtual ~Decoding();
    template <typename T, std::size_t n_bits>
    T get(bool big = false) {
        if (data->len - offset < n_bits) {
            throw Fail("decode fail with not enough data, expect %ld, but %ld", n_bits, data->len - offset);
        }
        T val;
        if (big) {
            val = boost::endian::detail::load_big_endian<T, n_bits>(data->data + offset);
        } else {
            val = boost::endian::detail::load_little_endian<T, n_bits>(data->data + offset);
        }
        offset += n_bits;
        return val;
    }
    void get(char *name, size_t len);
    void get(char *name, uint16_t &nlen);
    void get(char *name, uint16_t &nlen, char *value, uint16_t &vlen);
    void get(char *name, uint16_t &nlen, uint32_t &vlen);
    Data getstring(size_t len = 0);
    Data getdata(size_t len, bool iss = false);

    void inflate();

   public:
    Data data;
    size_t offset;
};

/*
 the ed2k login frame
 */
class Login {
   public:
    Hash hash;
    uint32_t cid;
    uint16_t port;
    Data name;
    uint32_t port2;        // 0x0F
    uint32_t version;      // 0x11
    uint32_t flags;        // 0x20
    uint32_t mver = 0xc8;  // 0xfb

   protected:
   public:
    Login();
    Login(Hash &hash, Data &name, uint32_t cid = 0, uint16_t port = 4662, uint32_t version = 0x3C,
          uint32_t flags = 0x319);
    virtual Data encode();
    virtual void parse(Data &data);
    virtual size_t show(char *buf = 0);
};
/*
 the ed2k server message
 */
class SrvMessage {
   public:
    Data msg;

   public:
    SrvMessage();
    SrvMessage(const char *msg, size_t len = 0);
    virtual Data encode();
    virtual void parse(Data &data);
    virtual const char *c_str();
};
/*
 the ed2k client ID
 */
class IDCHANGE {
   public:
    uint32_t id;
    uint32_t bitmap;

   public:
    IDCHANGE(uint32_t id = 0, uint32_t bitmap = 0x00000001);
    virtual Data encode();
    virtual void parse(Data &data);
};
/*
 the ed2k server status
 */
class SrvStatus {
   public:
    uint32_t userc;
    uint32_t filec;

   public:
    SrvStatus(uint32_t userc = 0, uint32_t filec = 0);
    virtual Data encode();
    virtual void parse(Data &data);
};
class FTagParser {
   public:
    uint8_t type = 0;
    uint8_t uname = 0;
    Data sname;
    //
    uint8_t u8v = 0;
    uint16_t u16v = 0;
    uint32_t u32v = 0;
    uint64_t u64v = 0;
    bool bval;
    Data sval;

   public:
    virtual void parse(Decoding &dec);
};
/*
 the ed2k file entry to upload
 */
class FileEntry_;
typedef boost::shared_ptr<FileEntry_> FileEntry;

class FileEntry_ : public boost::enable_shared_from_this<FileEntry_> {
   public:
    Hash hash;
    uint32_t cid = 0;
    uint16_t port = 0;
    Data name;              // 0x1
    uint64_t size = 0;      // 0x2
    Data type;              // 0x3
    Data format;            // 0x4
    uint8_t sources = 0;    // 0x15
    uint8_t completed = 0;  // 0x15
    Data artist;            // artist
    Data album;             // album;
    Data title;             // title
    uint32_t length = 0;    // length
    uint32_t bitrate = 0;   // bitrate
    uint32_t codec = 0;     // codec
    uint32_t gapstart = 0;  // 0x09
   public:
    FileEntry_();
    virtual ~FileEntry_();
    virtual void parse(Decoding &dec, uint8_t magic);
    std::string shash();
    virtual FileEntry share();
    virtual void print();
};
FileEntry BuildFileEntry();
/*
 the ed2k file list to upload
 */
class FileList {
   public:
    //    uint32_t filec;
    std::vector<FileEntry> fs;

   public:
    FileList();
    //    virtual ~FileList();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic);
};
/*
 the ed2k list server frame
 */
Data ListServer();
/*
 the ed2k server Address
 */
typedef std::pair<uint32_t, uint16_t> Address;
std::string addr_cs(Address &addr);
/*
 the ed2k server
 */
class ServerList {
   public:
    //        uint8_t count;
    std::list<Address> srvs;

   public:
    ServerList();
    virtual Data encode();
    virtual void parse(Data &data);
};

class ServerIndent {
   public:
    Hash hash;
    uint32_t addr = 0;
    uint16_t port = 0;
    Data name;  // 0x1
    Data desc;  // 0xB
   public:
    ServerIndent();
    ServerIndent(Hash &hash, uint32_t addr, uint32_t port, Data &name, Data &desc);
    virtual Data encode();
    virtual void parse(Data &data);
    std::string tostr();
};

class SearchArgs {
   public:
    Data search;
    Data type;
    Data max;
    Data min;
    Data avaiable;
    Data ext;

   public:
    SearchArgs();
    SearchArgs(const char *search);
    virtual Data encode();
    virtual void parse(Data &data);
};

class GetSource {
   public:
    Hash hash;
    uint64_t size;

   public:
    GetSource();
    GetSource(Hash &hash, uint64_t size);
    virtual Data encode();
    virtual void parse(Data &data);
};

class FoundSource {
   public:
    Hash hash;
    std::vector<Address> srvs;

   public:
    FoundSource();
    virtual Data encode();
    virtual void parse(Data &data);
};

class CallbackRequest {
   public:
    uint32_t cid;
    CallbackRequest(uint32_t cid = 0);
    virtual Data encode();
    virtual void parse(Data &data);
};

class CallbackRequested : public Address {
   public:
    CallbackRequested();
    virtual Data encode();
    virtual void parse(Data &data);
};

//////////c2c protocol//////////
class Hello {
   public:
    uint8_t magic;
    Hash hash;
    uint32_t cid;
    uint16_t port;
    Data name;       // 0x01
    Data version;    // 0x11
    uint16_t port2;  // 0x0f
    uint32_t saddr;
    uint16_t sport;

   public:
    Hello(uint8_t magic);
    virtual Data encode();
    virtual void parse(Data &data);
    std::string tostr();
};

typedef std::pair<uint32_t, uint32_t> FilePart;
class RequestParts {
   public:
    Hash hash;
    std::vector<FilePart> parts;

   public:
    RequestParts();
    virtual void addPart(uint32_t beg, uint32_t end);
    virtual Data encode();
    virtual void parse(Data &data);
};

class SendingPart {
   public:
    Hash hash;
    uint32_t start = 0;
    uint32_t end = 0;
    Data part;

   public:
    SendingPart();
    virtual Data encode();
    virtual void parse(Data &data);
};
//////////end encoding//////////
}
}

#endif /* encoding_hpp */
