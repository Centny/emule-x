//
//  ed2k_protocol.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef ed2k_protocol_hpp
#define ed2k_protocol_hpp
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
#include "../encoding/encoding.hpp"
#include "../fs/fs.hpp"
namespace emulex {
namespace protocol {
namespace ped2k {
using namespace boost;
using namespace butils::netw;
using namespace emulex::fs;
using namespace emulex::encoding;
//
#include "ed2k_opcodes.h"
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

//
#define S_USR_NAME_L 128
#define S_DESC_L 256

//
ModH BuildMod();

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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Decoding &dec, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class GetSource {
   public:
    Hash hash;
    uint64_t size;

   public:
    GetSource();
    GetSource(Hash &hash, uint64_t size);
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class FoundSource {
   public:
    Hash hash;
    std::vector<Address> srvs;

   public:
    FoundSource();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class CallbackRequest {
   public:
    uint32_t cid;
    CallbackRequest(uint32_t cid = 0);
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class CallbackRequested : public Address {
   public:
    CallbackRequested();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
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
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class MagicHash {
   public:
    uint8_t magic;
    Hash hash;

   public:
    MagicHash(uint8_t magic);
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class UploadRequest : public MagicHash {
   public:
    UploadRequest() : MagicHash(OP_STARTUPLOADREQ) {}
};

class HashsetRequest : public MagicHash {
   public:
    HashsetRequest() : MagicHash(OP_HASHSETREQUEST) {}
};

class FidRequest : public MagicHash {
   public:
    FidRequest() : MagicHash(OP_SETREQFILEID) {}
};

class FidAnswer {
   public:
    Hash hash;
    Data name;

   public:
    FidAnswer();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class HashsetAnswer {
   public:
    Hash hash;
    std::vector<Hash> parts;

   public:
    HashsetAnswer();
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class FileStatus {
   public:
    uint8_t magic;
    Hash hash;
    std::vector<uint8_t> parts;
    uint16_t source;

   public:
    FileStatus(uint8_t magic);
    virtual Data encode();
    virtual void parse(Data &data, uint8_t magic = OP_EDONKEYPROT);
};

class FileStatusRequest : public FileStatus {
   public:
    FileStatusRequest() : FileStatus(OP_REQUESTFILENAME) {}
};

class FileStatusAnswer : public FileStatus {
   public:
    FileStatusAnswer() : FileStatus(OP_REQFILENAMEANSWER) {}
};

class FileNotAnswer : public FileStatus {
   public:
    FileNotAnswer() : FileStatus(OP_FILEREQANSNOFIL) {}
};

//////////end protocol//////////
}
}
}

#endif /* ed2k_protocol */
