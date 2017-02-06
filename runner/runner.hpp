//
//  runner.hpp
//  emule-x
//
//  Created by Centny on 1/31/17.
//
//

#ifndef runner_hpp
#define runner_hpp
#include <json-c/json.h>
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
class Runner_;
class REvn_ {
   public:
    virtual void OnProcess(Runner_ &r, FTask task, int cons, float speed){};
    virtual void OnDone(Runner_ &r, FTask task){};
};
typedef boost::shared_ptr<REvn_> REvn;
//
class Sending_ {
   public:
    //    Hash hash;
    FTask task;
    std::vector<Part> parts;
    size_t sended = 0;
    File file;
    std::map<uint64_t, uint64_t> requested;

   public:
    std::vector<FilePart> ed2kpart(uint64_t cid);
};
typedef boost::shared_ptr<Sending_> Sending;
//
class Runner_ : public ed2k::Evn_, public boost::enable_shared_from_this<Runner_> {
   public:
   public:
    REvn H;
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
    std::map<FUUID, Sending, FUUIDComparer> singf;
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
    virtual FTask addTask(Data &location, HashType type, Hash &emd4, Data &filename, size_t size);
    virtual FTask addTask(const FUUID &uuid);
    virtual Sending down(const FUUID &uuid);
    virtual void release();

   protected:
    virtual void sendDone(ed2k::ED2K_ &ed2k, uint64_t cid);
    virtual Sending open(const FUUID &uuid);
};
typedef boost::shared_ptr<Runner_> Runner;
//
class WsWrapper_ {
   public:
    WS ws;
    Runner runner;

   protected:
    Reply mcode(int code, std::string msg);

   public:
    WsWrapper_(WS ws, Runner runner);
    Reply addTask(Con, Args);
    Reply listTask(Con, Args);

   public:
    virtual void OnProcess(Runner_ &r, FTask task, int cons, float speed);
    virtual void OnDone(Runner_ &r, FTask task);
};
typedef boost::shared_ptr<WsWrapper_> WsWrapper;
//
const int SPT_SEARCH_SRV=1;
const int SPT_PUBLISH_SRV=1<<1;
const int SPT_SEARCH_RES=1<<2;
//
}
}

#endif /* runner_hpp */
