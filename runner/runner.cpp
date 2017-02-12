//
//  runner.cpp
//  emule-x
//
//  Created by Centny on 1/31/17.
//
//

#include "runner.hpp"
namespace emulex {
namespace runner {

std::vector<FilePart> Sending_::ed2kpart(uint64_t cid) {
    std::vector<FilePart> ps;
    for (size_t i = sended; i < parts.size() && ps.size() < 3; i++) {
        ps.push_back(FilePart(parts[i].first, parts[i].second));
        sended++;
        requested[cid] = parts[i].second;
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
    : ios(ios), uuid(uuid), name(name), fmgr(fm) {
    H = REvn(new REvn_);
}

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
    auto tasks = fmgr->ts->listTask(FTSS_RUNNING);
    BOOST_FOREACH (const FTask& task, tasks) {
        try {
            auto ss = open(task);
            ed2k.listSource(cid, ss->task->emd4, ss->task->size, ecode);
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
    auto uuid = BuildFUUID(fs.hash, EMD4);
    File file = fmgr->findOpenedF(uuid);
    if (!file.get()) {
        V_LOG_W("Runner_ found source ignore hash(%s) by not opened task", fs.hash.tostring().c_str());
        return;
    }
    if (singf.find(uuid) == singf.end()) {
        V_LOG_W("Runner_ found source ignore hash(%s) by sending task not found", fs.hash.tostring().c_str());
        return;
    }
    BOOST_FOREACH (const Address& addr, fs.srvs) {
        auto con = ed2k.connect(ed2k::ed2k_c2c, addr.first, addr.second, ecode, 0, 0, cid);
        sending[con->Id()] = singf[uuid];
    }
}

void Runner_::OnAnswered(ed2k::ED2K_& ed2k, uint64_t cid) {
    //            std::vector<uint8_t> status;
    //            ed2k.rfilestatus(cid, sending[cid], status, 0, ecode);
    if (sending.find(cid) != sending.end()) {
        ed2k.uprequest(cid, sending[cid]->file->fc->emd4, ecode);
    }
    //                    ed2k.hashset(cid, entry->hash, ecode);
}

void Runner_::OnUpAccepted(ed2k::ED2K_& ed2k, uint64_t cid) {
    if (sending.find(cid) != sending.end()) {
        auto ss = sending[cid];
        auto ps = ss->ed2kpart(cid);
        if (ps.size() < 1) {
            return;
        }
        V_LOG_D("Runner_ sending hash(%s) parts(%u,%u,%u,%u,%u,%u),sended(%u),max(%u)",
                ss->file->fc->emd4.tostring().c_str(), ps[0].first, ps[0].second, ps[1].first, ps[1].second,
                ps[2].first, ps[2].second, ss->sended, ss->parts[ss->sended - 1].second);
        if (ps.size()) {
            ed2k.rfilepart(cid, ss->file->fc->emd4, ps, ecode);
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
    V_LOG_D("Runner_ receive file(%s) part(%lu,%lu) done(%d)", ss->file->fc->emd4.tostring().c_str(), part.start,
            part.part->len, done);
    if (done) {
        if (ss->file->valid(EMD4)) {
            V_LOG_D("Runner_ valid file(%s) passed", ss->file->fc->emd4.tostring().c_str());
            sendDone(ed2k, cid);
        } else {
            V_LOG_D("Runner_ valid file(%s) fail", ss->file->fc->emd4.tostring().c_str());
        }
        return;
    }
    if ((part.start + part.part->len) >= ss->requested[cid]) {  // requested done
        V_LOG_D("Runner_ file(%s) one part done by sended(%u,%u)", ss->file->fc->emd4.tostring().c_str(), ss->sended,
                ss->parts[ss->sended - 1].second);
        OnUpAccepted(ed2k, cid);
    }
}

void Runner_::sendDone(ed2k::ED2K_& ed2k, uint64_t cid) {
    auto ss = sending[cid];
    sending.erase(cid);
    fmgr->done(ss->file->fc);
    if (ss->task.get()) {
        fmgr->ts->updateTask(ss->task->tid, FTSS_DONE);
    }
}

void Runner_::OnHashsetAnswer(ed2k::ED2K_& ed2k, uint64_t cid, HashsetAnswer& hs) {}
void Runner_::OnFidAnswer(ed2k::ED2K_& ed2k, uint64_t cid, FidAnswer& fid) {}
void Runner_::OnFileStatusAnswer(ed2k::ED2K_& ed2k, uint64_t cid, FileStatus& status) {}

//
FTask Runner_::addTask(Data& location, HashType type, Hash& hash, Data& filename, size_t size) {
    FUUID uuid = BuildFUUID(hash, type);
    uuid->location = location;
    uuid->filename = filename;
    uuid->size = size;
    return addTask(uuid);
}

FTask Runner_::addTask(const FUUID& uuid) {
    auto task = fmgr->ts->addTask(uuid);
    auto ss = down(uuid);
    ss->task = task;
    V_LOG_I("Runner_ adding task on dir(%s) by hash(%s),filename(%s),size(%lu)", uuid->location->data,
            uuid->emd4.tostring().c_str(), uuid->filename->data, uuid->size);
    return task;
}

Sending Runner_::down(const FUUID& uuid) {
    auto ss = open(uuid);
    if (ss->file->fc->emd4.get()) {
        typedef std::map<uint64_t, uint64_t>::value_type mv;
        BOOST_FOREACH (const mv& v, logined) {
            ed2k->listSource(v.first, ss->file->fc->emd4, ss->file->fc->size, ecode);
        }
    }
    return ss;
}

void Runner_::release() { ed2k->release(); }

Sending Runner_::open(const FUUID& uuid) {
    Sending ss;
    if (singf.find(uuid) == singf.end()) {
        printf("%s\n", "ssss-00");
        ss = Sending(new Sending_);
        ss->file = fmgr->open(uuid);
        ss->parts = ss->file->split(EMBLOCKSIZE);
        ss->sended = 0;
        singf[uuid] = ss;
    } else {
        ss = singf[uuid];
    }
    return ss;
}

//
WsWrapper_::WsWrapper_(WS ws, Runner runner) {
    this->ws = ws;
    this->runner = runner;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;
    using websocketpp::lib::bind;
    this->ws->regh("/api/addTask", bind(&WsWrapper_::addTask_h, *this, _1, _2));
    this->ws->regh("/api/listTask", bind(&WsWrapper_::listTask_h, *this, _1, _2));
    this->ws->regm("addTask", bind(&WsWrapper_::addTask_m, *this, _1, _2, _3));
    this->ws->regm("listTask", bind(&WsWrapper_::listTask_m, *this, _1, _2, _3));
}

Reply WsWrapper_::mcode(int code, std::string msg) {
    std::stringstream ss;
    ss << "{\"code\":" << code << ",\"err\":\"" + msg + "\"}";
    return OkReply(ss.str());
}

Reply WsWrapper_::addTask_h(Con con, Args args) {
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
        auto ss = addTask(hash, type, dir, filename, size);
        return OkReply(ss);
    } catch (const std::exception& e) {
        V_LOG_E("addTaskH fail with %s by query->%s", e.what(), con->get_resource().c_str());
        return mcode(-1, e.what());
    } catch (...) {
        V_LOG_E("addTaskH fail with unknow error by query->%s", con->get_resource().c_str());
        return mcode(-1, "addTask fail with unknow error by query->" + con->get_resource());
    }
}

std::string WsWrapper_::addTask_m(HDL hdl, std::string& cmd, std::string& data) {
    struct json_object* obj;
    try {
        Hash hash;
        std::string type;
        std::string dir;
        std::string filename;
        size_t size;
        obj = json_tokener_parse(data.c_str());
        json_object_object_foreach(obj, key, val) {
            if (strcmp(key, "hash") == 0 && json_object_is_type(val, json_type_string)) {
                hash = FromHex(json_object_get_string(val));
            } else if (strcmp(key, "type") == 0 && json_object_is_type(val, json_type_string)) {
                type = std::string(json_object_get_string(val));
            } else if (strcmp(key, "dir") == 0 && json_object_is_type(val, json_type_string)) {
                dir = std::string(json_object_get_string(val));
            } else if (strcmp(key, "filename") == 0 && json_object_is_type(val, json_type_string)) {
                filename = std::string(json_object_get_string(val));
            } else if (strcmp(key, "size") == 0 && json_object_is_type(val, json_type_int)) {
                size = json_object_get_int(val);
            }
        }
        if (hash.get() == 0 || type.size() == 0 || dir.size() == 0 || filename.size() == 0) {
            json_object_put(obj);
            return std::string("the hash/type/dir/filename must not be empty");
        }
        auto ss = addTask(hash, type, dir, filename, size);
        json_object_put(obj);
        return ss;
    } catch (const std::exception& e) {
        json_object_put(obj);
        V_LOG_E("addTaskM fail with %s", e.what());
        return std::string(e.what());
    } catch (...) {
        json_object_put(obj);
        V_LOG_E("addTaskM fail with %s", "unknow error");
        return std::string("unknow error");
    }
}

std::string WsWrapper_::addTask(Hash& hash, std::string& type, std::string& dir, std::string& filename, size_t size) {
    HashType rtype;
    if (type == "emd4") {
        if (size < 1) {
            throw Fail("%s", "the size argument must not be zero when hash is emd4");
        }
        rtype = EMD4;
    } else if (type == "md5") {
        rtype = MD5;
    } else if (type == "sha1") {
        rtype = SHA1;
    } else {
        throw Fail("the type must be in emd4/md5/sha1, but %s", type.c_str());
    }
    auto fnc = BuildData(filename.c_str(), filename.size());
    auto dirc = BuildData(dir.c_str(), dir.size());
    auto task = runner->addTask(dirc, rtype, hash, fnc, size);
    auto jroot = json_object_new_object();
    json_object_object_add(jroot, "tid", json_object_new_int64(task->tid));
    json_object_object_add(jroot, "code", json_object_new_int(0));
    auto ss = std::string(json_object_to_json_string(jroot));
    json_object_put(jroot);
    return ss;
}

Reply WsWrapper_::listTask_h(Con con, Args args) {
    try {
        int status = FTSS_RUNNING | FTSS_DONE;
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
        auto ss = listTask(status, skip, limit);
        return OkReply(ss);
    } catch (const std::exception& e) {
        V_LOG_E("listTask fail with %s by query->%s", e.what(), con->get_resource().c_str());
        return mcode(-1, e.what());
    } catch (...) {
        V_LOG_E("listTask fail with unknow error by query->%s", con->get_resource().c_str());
        return mcode(-1, "addTask fail with unknow error by query->" + con->get_resource());
    }
}

std::string WsWrapper_::listTask_m(HDL hdl, std::string& cmd, std::string& data) {
    struct json_object* obj;
    try {
        int status = FTSS_RUNNING | FTSS_DONE;
        int skip = 0;
        int limit = 30;
        auto obj = json_tokener_parse(data.c_str());
        json_object_object_foreach(obj, key, val) {
            if (strcmp(key, "status") == 0 && json_object_is_type(val, json_type_int)) {
                status = json_object_get_int(val);
            } else if (strcmp(key, "skip") == 0 && json_object_is_type(val, json_type_int)) {
                skip = json_object_get_int(val);
            } else if (strcmp(key, "limit") == 0 && json_object_is_type(val, json_type_int)) {
                limit = json_object_get_int(val);
            }
        }
        auto ss = listTask(status, skip, limit);
        json_object_put(obj);
        return ss;
    } catch (const std::exception& e) {
        json_object_put(obj);
        V_LOG_E("addTaskM fail with %s", e.what());
        return std::string(e.what());
    } catch (...) {
        json_object_put(obj);
        V_LOG_E("addTaskM fail with %s", "unknow error");
        return std::string("unknow error");
    }
}

std::string WsWrapper_::listTask(int status, int skip, int limit) {
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
    return ss;
}

void WsWrapper_::OnProcess(Runner_& r, FTask task, int cons, float speed) {}

void WsWrapper_::OnDone(Runner_& r, FTask task) {}
//
}
}
