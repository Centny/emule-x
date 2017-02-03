//
//  runner.hpp
//  emule-x
//
//  Created by Centny on 1/31/17.
//
//

#ifndef runner_hpp
#define runner_hpp
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <list>
#include <set>
#include "../ed2k/ed2k.hpp"
#include "../fs/fs.hpp"
#include "../ws/ws.hpp"

namespace emulex {
namespace runner {
using namespace emulex::fs;
using namespace emulex::ws;
using namespace butils::netw;
using namespace emulex::protocol;
class Sending_ {
   public:
    Hash hash;
    std::vector<Part> parts;
    size_t sended = 0;
    File file;

   public:
    std::vector<FilePart> ed2kpart();
};
typedef boost::shared_ptr<Sending_> Sending;
//
class Runner_ : public ed2k::Evn_, public boost::enable_shared_from_this<Runner_> {
   public:
   public:
    Hash uuid;
    Data name;
    ed2k::ED2K ed2k;
    asio::io_service &ios;
    boost::system::error_code ecode;
    //
    std::map<uint64_t, uint64_t> logined;
    std::map<Address, int> allsrvs;
    std::map<Address, int> connected;
    std::map<Address, int> fail;
    std::map<uint64_t, Sending> sending;
    // w
    FileManager fmgr;

   public:
    Runner_(asio::io_service &ios, Hash &uuid, Data &name, FileManager &fm);
    virtual void start(std::list<Address> srvs);

   public:
    virtual void OnFail(ed2k::ED2K_ &ed2k, Address &addr);
    virtual void OnServerList(ed2k::ED2K_ &ed2k, uint64_t cid, ServerList srvs);
    virtual void OnLogined(ed2k::ED2K_ &ed2k, uint64_t cid, uint32_t lid);
    virtual void OnFoundFile(ed2k::ED2K_ &ed2k, uint64_t cid, FileList &fs);
    virtual void OnFoundSource(ed2k::ED2K_ &ed2k, uint64_t cid, FoundSource &fs);
    virtual void OnAnswered(ed2k::ED2K_ &ed2k, uint64_t cid);
    virtual void OnUpAccepted(ed2k::ED2K_ &ed2k, uint64_t cid);
    virtual void OnSending(ed2k::ED2K_ &ed2k, uint64_t cid, SendingPart &part);
    virtual void OnHashsetAnswer(ed2k::ED2K_ &ed2k, uint64_t cid, HashsetAnswer &hs);
    virtual void OnFidAnswer(ed2k::ED2K_ &ed2k, uint64_t cid, FidAnswer &fid);
    virtual void OnFileStatusAnswer(ed2k::ED2K_ &ed2k, uint64_t cid, FileStatus &status);

   public:
    virtual FTask addTask(boost::filesystem::path dir, Hash &hash, Data &filename, size_t size);
    virtual FTask addTask(boost::filesystem::path dir, FData &file);
    virtual File startTask(boost::filesystem::path dir, FData &file);

   protected:
    virtual void sendDone(ed2k::ED2K_ &ed2k, uint64_t cid);
};
typedef boost::shared_ptr<Runner_> Runner;
//
class WsWrapper_ {
   public:
    Runner runner;

   protected:
    Reply mcode(int code, std::string msg);

   public:
    WsWrapper_(Runner runner);
    Reply addTask(Con, Args);
    Reply listTask(Con, Args);
    void hand(WS ws);
};
typedef boost::shared_ptr<WsWrapper_> WsWrapper;
//
}
}

#endif /* runner_hpp */
