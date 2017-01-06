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
const uint8_t PROTOCOL = 0xE3;
//
const uint8_t OP_LOGINREQUEST = 0x01;
const uint8_t OP_IDCHANGE = 0x40;
const uint8_t OP_SERVERMESSAGE = 0x38;
const uint8_t OP_SERVERSTATUS = 0x34;
const uint8_t OP_OFFERFILES = 0x15;
const uint8_t OP_LIST_SERVER = 0x14;
const uint8_t OP_SERVERLIST = 0x32;
const uint8_t OP_SERVERIDENT = 0x41;
const uint8_t OP_SEARCHREQUEST = 0x16;

const uint8_t STR_TAG = 0x2;
const uint8_t INT_TAG = 0x3;
const uint8_t FLOAT_TAG = 0x4;

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
    Encoding &put(uint8_t val);
    Encoding &put(uint16_t val);
    Encoding &put(uint32_t val);
    Encoding &put(uint64_t val);
    //    Encoding& put(float val);
    Encoding &putv(const char *name, const char *val);
    Encoding &putv(uint8_t name, const char *val);
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
    int get(T &val) {
        if (data->len - offset < n_bits) {
            return -1;
        }
        val = boost::endian::detail::load_little_endian<T, n_bits>(data->data + offset);
        offset += n_bits;
        return 0;
    }
    int get(char *name, size_t len);
    int get(char *name, uint16_t &nlen);
    int get(char *name, uint16_t &nlen, char *value, uint16_t &vlen);
    int get(char *name, uint16_t &nlen, uint32_t &vlen);

   public:
    Data data;
    size_t offset;
};

/*
 the ed2k login frame
 */
class Login : public Encoding {
   public:
    char hash[22];
    uint32_t cid;
    uint16_t port;
    char name[256];
    uint32_t port2;         // 0x0F
    uint32_t version;       // 0x11
    uint32_t flags;         // 0x20
    uint32_t other = 0xc8;  // 0xfb

   protected:
   public:
    Login();
    Login(const char *hash, const char *name, uint32_t cid = 0, uint16_t port = 4662, uint32_t version = 0x3C,
          uint32_t flags = 0x319);
    virtual Data encode();
    virtual int parse(Data &data);
    virtual size_t show(char *buf = 0);
};
/*
 the ed2k server message
 */
class SrvMessage : public Encoding {
   public:
    Data msg;

   public:
    SrvMessage();
    SrvMessage(const char *msg, size_t len = 0);
    virtual Data encode();
    virtual int parse(Data &data);
    virtual const char *c_str();
};
/*
 the ed2k client ID
 */
class IDCHANGE : public Encoding {
   public:
    uint32_t id;
    uint32_t bitmap;

   public:
    IDCHANGE(uint32_t id = 0, uint32_t bitmap = 0x00000001);
    virtual Data encode();
    virtual int parse(Data &data);
};
/*
 the ed2k server status
 */
class SrvStatus : public Encoding {
   public:
    uint32_t userc;
    uint32_t filec;

   public:
    SrvStatus(uint32_t userc = 0, uint32_t filec = 0);
    virtual Data encode();
    virtual int parse(Data &data);
};
/*
 the ed2k file entry to upload
 */
class FileEntry {
   public:
    char hash[16];
    uint32_t cid;
    uint16_t port;
    Data name;         // 0x1
    uint64_t size;     // 0x2
    char type[64];     // 0x3
    char format[64];   // 0x4
    Data artist;       // artist
    Data album;        // album;
    Data title;        // title
    uint32_t length;   // length
    uint32_t bitrate;  // bitrate
    char codec[64];    // codec
};
/*
 the ed2k file list to upload
 */
class OfferFile : public Encoding {
   public:
    //    uint32_t filec;
    std::list<FileEntry> files;

   public:
    OfferFile();
    virtual Data encode();
    virtual int parse(Data &data);
};
/*
 the ed2k list server frame
 */
Data ListServer();
/*
 the ed2k server Address
 */
typedef std::pair<uint32_t, uint16_t> Address;
/*
 the ed2k server
 */
class ServerList : public Encoding {
   public:
    //        uint8_t count;
    std::list<Address> srvs;

   public:
    ServerList();
    virtual Data encode();
    virtual int parse(Data &data);
};

class ServerIndent : public Encoding {
   public:
    char hash[16];
    uint32_t ip;
    uint16_t port;
    char name[128];  // 0x1
    char desc[256];  // 0xB
   public:
    ServerIndent();
    ServerIndent(const char *hash, uint32_t ip, uint32_t port, const char *name, const char *desc);
    virtual Data encode();
    virtual int parse(Data &data);
};

class SearchArgs : public Encoding {
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
    virtual int parse(Data &data);
};

class SearchResult : public Encoding {};
//////////end encoding//////////
}
}

#endif /* encoding_hpp */
