//
//  ed2k_console_test.cpp
//  emule-x
//
//  Created by Centny on 1/5/17.
//
//

#include <stdio.h>
#include "ed2k.hpp"

using namespace boost;
using namespace emulex::ed2k;

class Ed2kTestH : public Evn_ {
   public:
    boost::system::error_code ecode;

   public:
    void OnLogined(ED2K_& ed2k, uint64_t cid, uint32_t lid) { ed2k.search(cid, "waving", ecode); }
    void OnFoundFile(ED2K_& ed2k, uint64_t cid, FileList& fs) {
        printf("->%s->%s->%llu\n", fs.fs[0]->hash.tostring().c_str(), fs.fs[0]->name->data, fs.fs[0]->size);
        ed2k.listSource(cid, fs.fs[0]->hash, fs.fs[0]->size, ecode);
    }
    void OnFoundSource(ED2K_& ed2k, uint64_t cid, FoundSource& fs) {
        printf("->%s\n", addr_cs(fs.srvs[0]).c_str());
        //        ed2k.connect(ed2k_c2c, "127.0.0.1", 20001, ecode, 0, 0, cid);
        ed2k.connect(ed2k_c2c, fs.srvs[0].first, fs.srvs[0].second, ecode, 0, 0, cid);
    }
};
std::string valxx() { return std::string("abc"); }
const char* valcc(const char* v) { return std::string(v).c_str(); }
const char* valcc() {
    const char* x = "abc";
    return std::string(x).c_str();
}
int edk_console(int argc, char** argv) {
    printf("%0x\n", 0 << 17 | 50 << 10 | 0 << 7);
    auto xxd = valxx();
    asio::io_service ios;
    Data name = BuildData("testing", 7);
    Hash hash = BuildHash(16);
    Evn h = Evn(new Ed2kTestH);
    ED2K ed2k(new ED2K_(ios, hash, name, h));
    boost::system::error_code err;
    //    ed2k->connect("10.211.55.8", 4122, err);
    ed2k->connect(ed2k_c2s, "127.0.0.1", 20000, err);
    printf("ec:%d \n", err.value());
    ios.run();
    printf("%s\n", "test ed2k console done...");
}
