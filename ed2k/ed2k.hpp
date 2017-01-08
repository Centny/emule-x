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

#define CT_ED2K_C2S 0x0100

#define CT_ED2K_S 0x0110

#define CT_ED2K_C2C 0x0120
enum con_t {
    defalut = 0,
    ed2k_c2s = CT_ED2K_C2S,
    ed2k_s = CT_ED2K_S,
    ed2k_c2c = CT_ED2K_C2C,
};
std::string con_t_cs(con_t c);
class ED2K_ : public CmdH_, public ConH_, public boost::enable_shared_from_this<ED2K_> {
   public:
    asio::io_service &ios;
    char cbuf[102400];
    //
    char hash[16];
    char name[128];
    bool logined;
    //
    ModH mod;
    std::map<uint64_t, Acceptor> usrv;
    std::map<uint64_t, TCP> tcs;
    std::map<uint64_t, UDP> ucs;
    std::map<uint64_t, IDCHANGE> ids;
    SrvStatus status;
    ServerList srvs;
    ServerIndent sid;
    FileList fs;
    FoundSource found;
    //
    boost::system::error_code ecode;

   protected:
    virtual void remove(TCP s);

   public:
    ED2K_(asio::io_service &ios);
    virtual Connector connect(con_t tag, const char *addr, unsigned short port, boost::system::error_code &err);
    virtual size_t send(uint64_t cid, Encoding &enc, boost::system::error_code &ec);

   protected:
    //    virtual void OnSrvLogined();
    virtual bool OnConn(TCP s, const boost::system::error_code &ec);
    virtual void OnClose(TCP s, const boost::system::error_code &ec);
    virtual int OnCmd(Cmd c);
    //
    virtual size_t write(uint64_t cid, Data data, boost::system::error_code &ec);

   public:
    virtual void login(uint64_t cid, boost::system::error_code &ec);
    virtual void listServer(uint64_t cid, boost::system::error_code &ec);
    virtual void search(uint64_t cid, const char *key, boost::system::error_code &ec);
    virtual void listSource(uint64_t cid, const char *hash, uint64_t size, boost::system::error_code &ec);
    virtual void callback(uint64_t cid, uint32_t tcid, boost::system::error_code &ec);
};

//////////end ed2k//////////
}
}

#endif /* ed2k_hpp */
