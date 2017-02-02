//
//  runner.cpp
//  emule-x
//
//  Created by Centny on 1/31/17.
//
//

#include "runner.hpp"
#include <boost/foreach.hpp>

namespace emulex {
namespace runner {

std::vector<FilePart> Sending::ed2kpart() {
    std::vector<FilePart> ps;
    for (size_t i = sended; i < parts.size(); i++) {
        ps.push_back(FilePart(parts[i].first, parts[i].second));
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
        auto ss = Sending();
        ss.hash = fs.hash;
        ss.file = fmgr->findOpenedF(fs.hash);
        ss.parts = ss.file->split(102400);
        ss.sended = 0;
        sending[con->Id()] = ss;
    }
}

void Runner_::OnAnswered(ed2k::ED2K_& ed2k, uint64_t cid) {
    //            std::vector<uint8_t> status;
    //            ed2k.rfilestatus(cid, sending[cid], status, 0, ecode);
    if (sending.find(cid) != sending.end()) {
        ed2k.uprequest(cid, sending[cid].hash, ecode);
    }
    //                    ed2k.hashset(cid, entry->hash, ecode);
}
void Runner_::OnUpAccepted(ed2k::ED2K_& ed2k, uint64_t cid) {
    if (sending.find(cid) != sending.end()) {
        auto ss = sending[cid];
        auto ps = ss.ed2kpart();
        V_LOG_D("Runner_ sending hash(%s) parts(%lu,%lu,%lu,%lu,%lu,%lu)", ss.hash.tostring().c_str(),
                ss.parts[0].first, ss.parts[0].second, ss.parts[1].first, ss.parts[1].second, ss.parts[2].first,
                ss.parts[2].second);
        if (ps.size()) {
            ed2k.rfilepart(cid, ss.hash, ps, ecode);
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
    auto done = ss.file->write(part.start, part.part);
    //    if(done){
    ss.file->fs->flush();
    //    }
    V_LOG_D("Runner_ receive file(%s) part(%lu,%lu) done(%d)", ss.hash.tostring().c_str(), part.start, part.part->len,
            done);
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
        file->format = BuildData(ext.c_str(), ext.size());
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
}
}
