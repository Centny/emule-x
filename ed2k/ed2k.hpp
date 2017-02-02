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
#include "../protocol/ed2k_protocol.hpp"

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
class Evn_ {
   public:
    virtual void OnFail(ED2K_ &ed2k, Address &addr) = 0;
    virtual void OnServerList(ED2K_ &ed2k, uint64_t cid, ServerList srvs) = 0;
    virtual void OnLogined(ED2K_ &ed2k, uint64_t cid, uint32_t lid) = 0;
    virtual void OnFoundFile(ED2K_ &ed2k, uint64_t cid, FileList &fs) = 0;
    virtual void OnFoundSource(ED2K_ &ed2k, uint64_t cid, FoundSource &fs) = 0;
    virtual void OnAnswered(ED2K_ &ed2k, uint64_t cid) = 0;
    virtual void OnUpAccepted(ED2K_ &ed2k, uint64_t cid) = 0;
    virtual void OnSending(ED2K_ &ed2k, uint64_t cid, SendingPart &part) = 0;
    virtual void OnHashsetAnswer(ED2K_ &ed2k, uint64_t cid, HashsetAnswer &hs) = 0;
    virtual void OnFidAnswer(ED2K_ &ed2k, uint64_t cid, FidAnswer &fid) = 0;
    virtual void OnFileStatusAnswer(ED2K_ &ed2k, uint64_t cid, FileStatus &status) = 0;
};
typedef boost::shared_ptr<Evn_> Evn;
typedef std::pair<uint16_t, uint16_t> Port;
class ED2K_ : public CmdH_, public ConH_, public boost::enable_shared_from_this<ED2K_> {
   public:
    class Server {
       public:
        Address addr;
        Port port;
        IDCHANGE id;
        SrvStatus status;
        ServerIndent sid;
        uint16_t from;
        uint32_t r_lid;
    };
    asio::io_service &ios;
    char cbuf[102400];
    //
    Hash hash;
    Data name;
    bool logined;
    int showlog = 0;
    //
    ModH mod;
    Evn H;
    std::map<uint64_t, Acceptor> usrv;
    std::map<uint64_t, TCP> tcs;
    std::map<uint64_t, UDP> ucs;
    std::map<uint64_t, Server> esrv;
    //
    boost::system::error_code ecode;

   protected:
    virtual void remove(TCP s);

   public:
    ED2K_(asio::io_service &ios, Hash &hash, Data &name, Evn h);
    virtual Connector connect(con_t tag, uint32_t addr, uint16_t port, boost::system::error_code &err,
                              uint16_t lport = 0, uint16_t pport = 0, uint32_t from = 0);
    virtual Connector connect(con_t tag, const char *addr, uint16_t port, boost::system::error_code &err,
                              uint16_t lport = 0, uint16_t pport = 0, uint32_t from = 0);
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
    virtual void listSource(uint64_t cid, Hash &hash, uint64_t size, boost::system::error_code &ec);
    virtual void callback(uint64_t cid, uint32_t lid, boost::system::error_code &ec);
    virtual void hello(uint64_t cid, uint64_t from, boost::system::error_code &ec);
    virtual void uprequest(uint64_t cid, Hash &hash, boost::system::error_code &ec);
    virtual void rfilepart(uint64_t cid, Hash &hash, std::vector<FilePart> &parts, boost::system::error_code &ec);
    virtual void rfilepart(uint64_t cid, Hash &hash, FilePart &part, boost::system::error_code &ec);
    virtual void rfid(uint64_t cid, Hash &hash, boost::system::error_code &ec);
    virtual void rfilestatus(uint64_t cid, Hash &hash, std::vector<uint8_t> &status, uint16_t source,
                             boost::system::error_code &ec);
    virtual void hashset(uint64_t cid, Hash &hash, boost::system::error_code &ec);

   public:
    virtual void close(uint64_t cid);
};

//////////end ed2k//////////
}
}

#endif /* ed2k_hpp */
