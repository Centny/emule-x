//
//  ed2k.hpp
//  ed2k
//
//  Created by Centny on 1/1/17.
//
//

#ifndef ed2k_hpp
#define ed2k_hpp
#include <boost-utils/boost-utils.hpp>
#include "../protocol/encoding.hpp"

namespace emulex {
namespace ed2k {
using namespace butils::netw;
using namespace emulex::protocol;
class ED2K_;
typedef boost::shared_ptr<ED2K_> ED2K;

class ED2K_ : public CmdH_, public ConH_, public boost::enable_shared_from_this<ED2K_> {
   public:
    asio::io_service &ios;
    char cbuf[102400];
    //
    char hash[16];
    char name[128];
    bool logined;
    //
    Connector con;
    Acceptor srv;
    SrvMessage smsg;
    IDCHANGE id;
    SrvStatus status;
    ServerList srvs;
    ServerIndent sid;
    FileList fs;
    FoundSource found;
    //
    boost::system::error_code ecode;

   public:
    ED2K_(asio::io_service &ios);
    virtual void connect(const char *addr, unsigned short port, boost::system::error_code &err);
    virtual size_t send(Encoding &enc, boost::system::error_code &ec);

   protected:
    virtual void OnSrvLogined();
    virtual bool OnConn(TCP s, const boost::system::error_code &ec);
    // event on socket close.
    virtual void OnClose(TCP s, const boost::system::error_code &ec);
    virtual int OnCmd(Cmd c);

   public:
    virtual void login(boost::system::error_code &ec);
    virtual void listServer(boost::system::error_code &ec);
    virtual void search(const char *key, boost::system::error_code &ec);
    virtual void listSource(const char *hash, uint64_t size, boost::system::error_code &ec);
    virtual void callback(uint32_t cid, boost::system::error_code &ec);
};

//////////end ed2k//////////
}
}

#endif /* ed2k_hpp */
