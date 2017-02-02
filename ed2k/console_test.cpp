//
//  ed2k_console_test.cpp
//  emule-x
//
//  Created by Centny on 1/5/17.
//
//

#include <openssl/md4.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "ed2k.hpp"

using namespace boost;
using namespace emulex::ed2k;

class Ed2kTestH : public Evn_ {
   public:
    boost::system::error_code ecode;
    FileEntry entry;
    std::ofstream xfs;
    size_t offset;
    int count = 0;
    uint64_t bsize = 0;
    uint64_t os0, os0_, os1, os1_, os2, os2_;

   public:
    void OnFail(ED2K_& ed2k, Address& addr) {}
    void OnServerList(ED2K_& ed2k, uint64_t cid, ServerList srvs) {}
    void OnLogined(ED2K_& ed2k, uint64_t cid, uint32_t lid) { ed2k.search(cid, "Office", ecode); }
    void OnFoundFile(ED2K_& ed2k, uint64_t cid, FileList& fs) {
        printf("->%s->%s->%llu\n", fs.fs[0]->hash.tostring().c_str(), fs.fs[0]->name->data, fs.fs[0]->size);
        ed2k.listSource(cid, fs.fs[0]->hash, fs.fs[0]->size, ecode);
        entry = fs.fs[0];
        //        xfs.open(entry->name->data);
        //        ed2k.callback(cid, 6, ecode);
    }
    void OnFoundSource(ED2K_& ed2k, uint64_t cid, FoundSource& fs) {
        printf("found->%s\n", addr_cs(fs.srvs[0]).c_str());
        // ed2k.connect(ed2k_c2c, "127.0.0.1", 20001, ecode, 0, 0, cid);
        ed2k.connect(ed2k_c2c, fs.srvs[1].first, fs.srvs[1].second, ecode, 0, 0, cid);
    }
    void OnAnswered(ED2K_& ed2k, uint64_t cid) {
        bsize = entry->size / 3;
        os0 = 0, os0_ = bsize + 3;
        os1 = bsize + 3, os1_ = 2 * bsize + 3;
        os2 = 2 * bsize + 3, os2_ = entry->size;
        //        ed2k.uprequest(cid, entry->hash, ecode);
        //        ed2k.hashset(cid, entry->hash, ecode);
        //        ed2k.
    }
    void OnUpAccepted(ED2K_& ed2k, uint64_t cid) {
        printf("accepted->\n");
        std::vector<FilePart> parts;
        if (os0 < os0_) {
            if (os0_ - os0 > 102400) {
                parts.push_back(FilePart(os0, os0 + 102400));
                os0 += 102400;
            } else {
                parts.push_back(FilePart(os0, os0_));
                os0 = os0_;
            }
        }
        if (os1 < os1_) {
            if (os1_ - os1 > 102400) {
                parts.push_back(FilePart(os1, os1 + 102400));
                os1 += 102400;
            } else {
                parts.push_back(FilePart(os1, os1_));
                os1 = os1_;
            }
        }
        if (os2 < os2_) {
            if (os2_ - os2 > 102400) {
                parts.push_back(FilePart(os2, os2 + 102400));
                os2 += 102400;
            } else {
                parts.push_back(FilePart(os2, os2_));
                os2 = os2_;
            }
        }
        if (parts.size() == 3) {
            ed2k.rfilepart(cid, entry->hash, parts, ecode);
        } else {
            xfs.close();
        }
        count = 0;
    }
    void OnSending(ED2K_& ed2k, uint64_t cid, SendingPart& part) {
        printf("sending->%u-%u\n", part.start, part.end);
        xfs.seekp(part.start);
        xfs.write(part.part->data, part.part->len);
        count++;
        if (count > 2) {
            OnUpAccepted(ed2k, cid);
        }
    }
    void OnHashsetAnswer(ED2K_& ed2k, uint64_t cid, HashsetAnswer& hs) {}
    void OnFidAnswer(ED2K_& ed2k, uint64_t cid, FidAnswer& fid) {}
    void OnFileStatusAnswer(ED2K_& ed2k, uint64_t cid, FileStatusAnswer& fid) {}
    void OnFileStatusAnswer(ED2K_& ed2k, uint64_t cid, FileStatus& fid) {}
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
    ed2k->connect(ed2k_c2s, "14.23.162.173", 4122, err);
    //    ed2k->connect(ed2k_c2s, "127.0.0.1", 20000, err);
    printf("ec:%d \n", err.value());
    ios.run();
    printf("%s\n", "test ed2k console done...");
}

int ed2k_md4(int argc, char** argv) {
    char string[] = "hello world";

    MD4_CTX ctx;
    MD4_Init(&ctx);
    FILE* fid = fopen("/tmp/a.mp4", "r");
    char buf[9728];
    size_t readed = 1;
    while (readed) {
        MD4_CTX pctx;
        MD4_Init(&pctx);
        for (int i = 0; i < 1000; i++) {
            readed = fread(buf, 1, 9728, fid);
            if (readed < 1) {
                break;
            }
            MD4_Update(&pctx, buf, readed);
        }
        unsigned char digest[MD4_DIGEST_LENGTH];
        MD4_Final(digest, &pctx);
        for (int i = 0; i < MD4_DIGEST_LENGTH; i++) printf("%02x", digest[i]);
        printf("\n\n");
        MD4_Update(&ctx, digest, MD4_DIGEST_LENGTH);
    }
    unsigned char digest[MD4_DIGEST_LENGTH];
    MD4_Final(digest, &ctx);
    fclose(fid);
    // save md4 result into char array - doesnt work
    //    char test[MD4_DIGEST_LENGTH];
    //    sprintf(test, "%02x", (unsigned int)digest);
    //    for(int i = 0; i < MD4_DIGEST_LENGTH; i++)
    //        printf("%02x", test[i]);
    //    printf("\n\n");

    // print out md4 result - works, but its not intochar array as I wanted it to be
    for (int i = 0; i < MD4_DIGEST_LENGTH; i++) printf("%02x", digest[i]);
    printf("\n\n");

    // works but i dont understand why 'mdString' is 33 size
    char mdString[33];
    for (int i = 0; i < MD4_DIGEST_LENGTH; i++)
        // and I also dont get i*2 in this loop
        sprintf(&mdString[i * 2], "%02x", (unsigned int)digest[i]);
    printf("md4 digest: %s\n", mdString);

    return 0;
}
