//
//  kadx.hpp
//  emule-x
//
//  Created by Centny on 2/6/17.
//
//

#ifndef kadx_hpp
#define kadx_hpp
#include <boost-utils/boost-utils.hpp>
#include <boost/foreach.hpp>
#include "../encoding/encoding.hpp"
#include "../fs/fs.hpp"
#include "../protocol/kadx_protocol.hpp"

namespace emulex {
namespace kadx {
using namespace butils::netw;
using namespace emulex::fs;
using namespace emulex::encoding;
using namespace emulex::protocol;
class KADX_;
typedef boost::shared_ptr<KADX_> ED2K;

enum ConType { Normal = 0, C2S = 0x0100, C2C = 0x0110, S2C = 0x0120, S2S = 0x0130 };
//
std::string ConTypeS(ConType c);
//
class KADX_;
class Evn_ {
   public:
    virtual std::vector<FData> OnSearch(KADX_ &kadx, pkadx::Search &s) = 0;
    virtual void OnSearchBack(KADX_ &kadx, pkadx::SearchBack &back) = 0;
    virtual bool OnSearchSource(KADX_ &kadx, pkadx::SearchSource &ss) = 0;
    virtual void OnFoundSource(KADX_ &kadx, pkadx::FoundSource &fs) = 0;
    virtual pkadx::FilePart OnFileStatus(KADX_ &kadx, pkadx::FileStatus &fs) = 0;
    virtual void OnFileStatusBack(KADX_ &kadx, pkadx::FilePart &fs) = 0;
    virtual void OnFilePart(KADX_ &kadx, pkadx::FilePart &fs) = 0;
    virtual void OnFileProc(KADX_ &kadx, pkadx::FileProc &proc) = 0;
    //    virtual void OnHoled(KADX_ &kadx,)
};
typedef boost::shared_ptr<Evn_> Evn;
typedef std::pair<uint16_t, uint16_t> Port;
class KADX_ : public CmdH_, public ConH_, public boost::enable_shared_from_this<KADX_> {
   public:
    asio::io_service &ios;
    char cbuf[102400];
    //
    pkadx::Login uuid;
    int showlog = 0;
    uint16_t taskc = 1;
    //
    ModH mod;
    Evn H;
    std::map<uint64_t, Monitor> usrv;
    std::map<uint64_t, Acceptor> tsrv;
    std::map<uint64_t, TCP> tcs;
    std::map<uint64_t, UDP> ucs;
    //
    std::map<uint64_t, Writer> s2s;
    std::map<uint64_t, Writer> s2c;
    std::map<uint64_t, Writer> c2s;
    typedef std::map<uint64_t, Writer>::value_type TypeWriter;
    //
    std::map<uint64_t, pkadx::Login> logined;
    std::map<pkadx::Task, uint64_t, pkadx::TaskComparer> stask;
    std::map<uint32_t, Hash> holing;
    //
    boost::system::error_code ecode;

   protected:
    virtual void remove(TCP s);

   public:
    KADX_(asio::io_service &ios, pkadx::Login uuid, Evn h);
    virtual Connector connect(ConType tag, uint32_t addr, uint16_t port, boost::system::error_code &err,
                              uint16_t lport = 0, uint16_t pport = 0, uint32_t from = 0);
    virtual Connector connect(ConType tag, const char *addr, uint16_t port, boost::system::error_code &err,
                              uint16_t lport = 0, uint16_t pport = 0, uint32_t from = 0);
    virtual Acceptor listen(ConType tag, const char *addr, uint16_t port, bool reused, boost::system::error_code &err);
    virtual Monitor monitor(ConType tag, const char *addr, uint16_t port, bool reused, boost::system::error_code &err);
    virtual UDP conto(ConType tag, const char *addr, uint16_t port, bool reused, Data data,
                      boost::system::error_code &err);
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
    virtual void search(uint64_t cid, Data &key, boost::system::error_code &ec);
    virtual void ssrc(uint64_t cid, Hash &hash, uint64_t size, boost::system::error_code &ec);
    virtual void hole(uint64_t cid, Hash &hash, uint64_t tcid, boost::system::error_code &ec);
    virtual void rfilestatus(uint64_t cid, Hash &hash, boost::system::error_code &ec);
    virtual void rfilepart(uint64_t cid, Hash &hash, SortedPart &parts, boost::system::error_code &ec);
    //    virtual void listServer(uint64_t cid, boost::system::error_code &ec);
    //    virtual void search(uint64_t cid, const char *key, boost::system::error_code &ec);
    //    virtual void listSource(uint64_t cid, Hash &hash, uint64_t size, boost::system::error_code &ec);
    //    virtual void callback(uint64_t cid, uint32_t lid, boost::system::error_code &ec);
    //    virtual void hello(uint64_t cid, uint64_t from, boost::system::error_code &ec);
    //    virtual void uprequest(uint64_t cid, Hash &hash, boost::system::error_code &ec);
    //    virtual void rfilepart(uint64_t cid, Hash &hash, std::vector<FilePart> &parts, boost::system::error_code &ec);
    //    virtual void rfilepart(uint64_t cid, Hash &hash, FilePart &part, boost::system::error_code &ec);
    //    virtual void rfid(uint64_t cid, Hash &hash, boost::system::error_code &ec);
    //    virtual void rfilestatus(uint64_t cid, Hash &hash, std::vector<uint8_t> &status, uint16_t source,
    //                             boost::system::error_code &ec);
    //    virtual void hashset(uint64_t cid, Hash &hash, boost::system::error_code &ec);

   public:
    virtual void close(uint64_t cid);
    virtual void release();
};

//////////end kadx//////////
}
}

#endif /* kadx_hpp */
