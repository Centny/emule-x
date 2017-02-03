//
//  runner.cpp
//  emule-x
//
//  Created by Centny on 1/31/17.
//
//

#include "runner.hpp"
#include <json-c/json.h>
namespace emulex {
namespace runner {

std::vector<FilePart> Sending_::ed2kpart() {
    std::vector<FilePart> ps;
    for (size_t i = sended; i < parts.size() && ps.size() < 3; i++) {
        ps.push_back(FilePart(parts[i].first, parts[i].second));
        sended++;
    }
    if (ps.size() < 1) {
        return ps;
    }
    while (ps.size() < 3) {
        ps.push_back(FilePart(0, 0));
    }
    return ps;
}

Runner_::Runner_(asio::io_service& ios, Hash& uuid, Data& name, FileManager& fm)
    : ios(ios), uuid(uuid), name(name), fmgr(fm) {}

void Runner_::start(std::list<Address> srvs) {
    if (ed2k.get() == 0) {
        ed2k = ed2k::ED2K(new ed2k::ED2K_(ios, uuid, name, shared_from_this()));
    }
    boost::system::error_code err;
    BOOST_FOREACH (const Address& srv, srvs) {
        if (allsrvs[srv] > 0) {
            continue;
        }
        allsrvs[srv] = 1;
        ed2k->connect(ed2k::ed2k_c2s, srv.first, srv.second, err);
    }
}

void Runner_::OnFail(ed2k::ED2K_& ed2k, Address& addr) {
    fail[addr]++;
    connected.erase(addr);
}

void Runner_::OnServerList(ed2k::ED2K_& ed2k, uint64_t cid, ServerList srvs) { start(srvs.srvs); }

void Runner_::OnLogined(ed2k::ED2K_& ed2k, uint64_t cid, uint32_t lid) {
    logined[cid] = lid;
    connected[ed2k.esrv[cid].addr]++;
    auto tasks = fmgr->ts->listTask();
    BOOST_FOREACH (const FTask& task, tasks) {
        try {
            boost::filesystem::path dir(task->location->data);
            auto file = fmgr->open(dir, task->filename);
            ed2k.listSource(cid, file->fc->fd->emd4, file->fc->fd->size, ecode);
        } catch (const std::exception& exc) {
            V_LOG_E("Runner_.OnLogined to start task by location(%s),filename(%s) fail with %s", task->location->data,
                    task->filename->data, exc.what());
        }
    }
}

void Runner_::OnFoundFile(ed2k::ED2K_& ed2k, uint64_t cid, FileList& fs) {
    std::list<FData> fds;
    BOOST_FOREACH (const FileEntry& f, fs.fs) {
        auto fd = fmgr->fs->find(f->hash, 2);
        if (fd.get()) {
            if (fd->filename->cmp(f->name)) {
                continue;
            }
            fd->filename = JoinData(fd->filename, f->name);
            fmgr->fs->updateFilename(fd->tid, fd->filename);
        } else {
            fd = FData(new FData_);
            fd->emd4 = f->hash;
            fd->filename = f->name;
            fd->size = f->size;
            fd->format = f->format;
            fmgr->fs->add(fd);
        }
        fds.push_back(fd);
    }
}

void Runner_::OnFoundSource(ed2k::ED2K_& ed2k, uint64_t cid, FoundSource& fs) {
    File file = fmgr->findOpenedF(fs.hash);
    if (!file.get()) {
        V_LOG_I("Runner_ found source ignore hash(%s) by not opened task", fs.hash.tostring().c_str());
        return;
    }
    BOOST_FOREACH (const Address& addr, fs.srvs) {
        auto con = ed2k.connect(ed2k::ed2k_c2c, addr.first, addr.second, ecode, 0, 0, cid);
        auto ss = Sending(new Sending_);
        ss->hash = fs.hash;
        ss->file = fmgr->findOpenedF(fs.hash);
        ss->parts = ss->file->split(EMBLOCKSIZE);
        ss->sended = 0;
        sending[con->Id()] = ss;
    }
}

void Runner_::OnAnswered(ed2k::ED2K_& ed2k, uint64_t cid) {
    //            std::vector<uint8_t> status;
    //            ed2k.rfilestatus(cid, sending[cid], status, 0, ecode);
    if (sending.find(cid) != sending.end()) {
        ed2k.uprequest(cid, sending[cid]->hash, ecode);
    }
    //                    ed2k.hashset(cid, entry->hash, ecode);
}

void Runner_::OnUpAccepted(ed2k::ED2K_& ed2k, uint64_t cid) {
    if (sending.find(cid) != sending.end()) {
        auto ss = sending[cid];
        auto ps = ss->ed2kpart();
        V_LOG_D("Runner_ sending hash(%s) parts(%u,%u,%u,%u,%u,%u),sended(%u),max(%u)", ss->hash.tostring().c_str(), ps[0].first,
                ps[0].second, ps[1].first, ps[1].second, ps[2].first, ps[2].second, ss->sended,ss->parts[ss->sended - 1].second);
        if (ps.size()) {
            ed2k.rfilepart(cid, ss->hash, ps, ecode);
        } else {
            // ed2k.close(cid);
        }
    }
}

void Runner_::OnSending(ed2k::ED2K_& ed2k, uint64_t cid, SendingPart& part) {
    if (sending.find(cid) == sending.end()) {
        return;
    }
    auto ss = sending[cid];
    auto done = ss->file->write(part.start, part.part);
    V_LOG_D("Runner_ receive file(%s) part(%lu,%lu) done(%d)", ss->hash.tostring().c_str(), part.start, part.part->len,
            done);
    if (done) {
        if (ss->file->valid(EMD4)) {
            V_LOG_D("Runner_ valid file(%s) passed", ss->hash.tostring().c_str());
            sendDone(ed2k, cid);
        } else {
            V_LOG_D("Runner_ valid file(%s) fail", ss->hash.tostring().c_str());
        }
        return;
    }
    if ((part.start + part.part->len) >= ss->parts[ss->sended - 1].second) {
        V_LOG_D("Runner_ file(%s) one part done by sended(%u,%u)", ss->hash.tostring().c_str(),ss->sended, ss->parts[ss->sended - 1].second);
        if (ss->sended < ss->parts.size()) {
            OnUpAccepted(ed2k, cid);
        }
    }
}

void Runner_::sendDone(ed2k::ED2K_& ed2k, uint64_t cid) {
    auto ss = sending[cid];
    sending.erase(cid);
    fmgr->done(ss->hash);
}

void Runner_::OnHashsetAnswer(ed2k::ED2K_& ed2k, uint64_t cid, HashsetAnswer& hs) {}
void Runner_::OnFidAnswer(ed2k::ED2K_& ed2k, uint64_t cid, FidAnswer& fid) {}
void Runner_::OnFileStatusAnswer(ed2k::ED2K_& ed2k, uint64_t cid, FileStatus& status) {}

//
FTask Runner_::addTask(boost::filesystem::path dir, Hash& hash, Data& filename, size_t size) {
    FData file = FData(new FData_);
    file->emd4 = hash;
    file->filename = filename;
    file->size = size;
    auto pp = boost::filesystem::path(filename->data);
    if (pp.has_extension()) {
        auto ext = pp.extension();
        auto extc = ext.c_str();
        file->format = BuildData(extc, strlen(extc));
    }
    return addTask(dir, file);
}

FTask Runner_::addTask(boost::filesystem::path dir, FData& file) {
    auto task = fmgr->ts->addTask(dir, file);
    startTask(dir, file);
    V_LOG_I("Runner_ adding task on dir(%s) by hash(%s),filename(%s),size(%lu)", dir.c_str(),
            file->emd4.tostring().c_str(), file->filename->data, file->size);
    return task;
}
File Runner_::startTask(boost::filesystem::path dir, FData& file) {
    auto f = fmgr->open(dir, file);
    typedef std::map<uint64_t, uint64_t> mv;
    BOOST_FOREACH (const mv::value_type& v, logined) { ed2k->listSource(v.first, file->emd4, file->size, ecode); }
    return f;
}
//
WsWrapper_::WsWrapper_(Runner runner) { this->runner = runner; }

Reply WsWrapper_::mcode(int code, std::string msg) {
    std::stringstream ss;
    ss << "{\"code\":" << code << ",\"err\":\"" + msg + "\"}";
    return OkReply(ss.str());
}

Reply WsWrapper_::addTask(Con con, Args args) {
    try {
        if (args.find("hash") == args.end()) {
            return mcode(1, "hash argument is null or empty");
        }
        auto hash = FromHex(args["hash"].c_str());
        if (!hash) {
            return mcode(2, "parse hash by hex fail");
        }
        if (args.find("type") == args.end()) {
            return mcode(1, "type argument is null or empty");
        }
        auto type = args["type"];
        if (args.find("dir") == args.end()) {
            return mcode(1, "dir argument is null or empty");
        }
        auto dir = args["dir"];
        if (args.find("filename") == args.end()) {
            return mcode(1, "filename argument is null or empty");
        }
        auto filename = args["filename"];
        size_t size = 0;
        if (args.find("size") != args.end()) {
            size = boost::lexical_cast<size_t>(args["size"].c_str());
        }
        if (type == "emd4") {
            if (size < 1) {
                return mcode(1, "the size argument must not be zero when hash is emd4");
            }
            auto fnc = BuildData(filename.c_str(), filename.size());
            runner->addTask(boost::filesystem::path(dir), hash, fnc, size);
            return mcode(0, "OK");
        }
        return mcode(10, "type must be in emd4/md5/sha1");
    } catch (const std::exception& e) {
        V_LOG_E("addTask fail with %s by query->%s", e.what(), con->get_resource().c_str());
        return mcode(-1, e.what());
    } catch (...) {
        V_LOG_E("addTask fail with unknow error by query->%s", con->get_resource().c_str());
        return mcode(-1, "addTask fail with unknow error by query->" + con->get_resource());
    }
}

Reply WsWrapper_::listTask(Con con, Args args) {
    try {
        int status = FTSS_RUNNING;
        if (args.find("status") != args.end()) {
            status = boost::lexical_cast<int>(args["status"].c_str());
        }
        int skip = 0;
        if (args.find("skip") != args.end()) {
            skip = boost::lexical_cast<int>(args["skip"].c_str());
        }
        int limit = 30;
        if (args.find("limit") != args.end()) {
            limit = boost::lexical_cast<int>(args["limit"].c_str());
        }
        auto tasks = runner->fmgr->ts->listTask(status, skip, limit);
        auto jroot = json_object_new_object();
        auto jtasks = json_object_new_array();
        BOOST_FOREACH (const FTask& task, tasks) {
            auto jtask = json_object_new_object();
            json_object_object_add(jtask, "tid", json_object_new_int64(task->tid));
            json_object_object_add(jtask, "filename", json_object_new_string(task->filename->data));
            json_object_object_add(jtask, "location", json_object_new_string(task->location->data));
            json_object_object_add(jtask, "size", json_object_new_int64(task->size));
            json_object_object_add(jtask, "done", json_object_new_int64(task->done));
            json_object_object_add(jtask, "format", json_object_new_string(task->format->data));
            json_object_object_add(jtask, "used", json_object_new_int64(task->used));
            json_object_object_add(jtask, "status", json_object_new_int64(task->status));
            json_object_array_add(jtasks, jtask);
            // json_object_put(jtask);
        }
        json_object_object_add(jroot, "tasks", jtasks);
        // json_object_put(jtasks);
        json_object_object_add(jroot, "code", json_object_new_int(0));
        auto ss = std::string(json_object_to_json_string(jroot));
        json_object_put(jroot);
        return OkReply(ss);
    } catch (const std::exception& e) {
        V_LOG_E("listTask fail with %s by query->%s", e.what(), con->get_resource().c_str());
        return mcode(-1, e.what());
    } catch (...) {
        V_LOG_E("listTask fail with unknow error by query->%s", con->get_resource().c_str());
        return mcode(-1, "addTask fail with unknow error by query->" + con->get_resource());
    }
}

void WsWrapper_::hand(WS ws) {
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;
    ws->reg("/api/addTask", bind(&WsWrapper_::addTask, this, _1, _2));
    ws->reg("/api/listTask", bind(&WsWrapper_::listTask, this, _1, _2));
}
//
}
}
