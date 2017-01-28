//
//  fs_test.hpp
//  emule-x
//
//  Created by Centny on 1/11/17.
//
//

#ifndef fs_test_h
#define fs_test_h

#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include "fs.hpp"

using namespace boost;

BOOST_AUTO_TEST_SUITE(FS)  // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE(TestHash) {
    char data[] = {
        0x4b, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    char data2[] = {
        0x4c, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    char data3[] = {
        0x4c, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2,
    };
    Hash h1 = BuildHash(data, 16);
    Hash h2 = BuildHash(data, 16);
    Hash h3 = BuildHash(data2, 16);
    Hash h4 = BuildHash(data3, 13);
    BOOST_CHECK_EQUAL(h1 == h2, true);
    BOOST_CHECK_EQUAL(h1.tostring() == h2.tostring(), true);
    BOOST_CHECK_EQUAL(h2 == h3, false);
    BOOST_CHECK_EQUAL(h3 == h4, false);
    h2.reset();
    BOOST_CHECK_EQUAL(h1 == h2, false);
    h1.reset();
    BOOST_CHECK_EQUAL(h1 == h2, false);
    Hash h5 = BuildHash(16);
    BOOST_CHECK_EQUAL(h1 == h5, false);
    std::list<Hash> hs;
    {
        hs.push_back(BuildHash(data, 16));
        hs.push_back(BuildHash(data, 16));
    }
    hs.clear();
    h1.set(100);
    h1.set("abc", 3);
    printf("TestHash done...\n");
}

BOOST_AUTO_TEST_CASE(TestHashFree) {
    char data[] = {
        0x4b, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    char data2[] = {
        0x4c, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    std::list<Hash> hs;
    {
        hs.push_back(BuildHash(data, 16));
        hs.push_back(BuildHash(data2, 16));
    }
    {
        Hash h1 = hs.front();
        Hash h2 = hs.back();
        BOOST_CHECK_EQUAL(h1 == h2, false);
        BOOST_CHECK_EQUAL(h1.tostring() == h2.tostring(), false);
    }
    hs.clear();
    printf("TestHashFree done...\n");
}

BOOST_AUTO_TEST_CASE(TestHashMap) {
    char data[] = {
        0x4b, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    char data2[] = {
        0x4c, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2, 0x28, 0x40, 0xfd,
    };
    char data3[] = {
        0x4c, 0x7a, 0x86, 0x00, 0x28, 0x14, 0x76, 0x60, 0x2e, 0x9c, 0x71, 0x11, 0xb2,
    };
    std::map<Hash, int> hs;
    {
        Hash h1 = BuildHash(data, 16);
        Hash h2 = BuildHash(data, 16);
        Hash h3 = BuildHash(data2, 16);
        Hash h4 = BuildHash(data3, 13);
        hs[h1] = 1;
        hs[h1] = 2;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 1);
        hs[h2] = 1;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 1);
        hs[h3] = 1;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 2);
        hs[h4] = 1;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 3);
        //
        Hash h5 = BuildHash(data, 16);
        Hash h6 = BuildHash(data, 16);
        BOOST_CHECK_EQUAL(h5 < h6, false);
        h6.reset();
        BOOST_CHECK_EQUAL(h5 < h6, true);
        h5.reset();
        BOOST_CHECK_EQUAL(h5 < h6, true);
    }
    hs.clear();
    printf("TestHashMap done...\n");
}

BOOST_AUTO_TEST_CASE(TestHax) {
    std::map<std::string, int> hs;
    {
        hs[std::string("a")] = 1;
        hs[std::string("b")] = 2;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 2);
        hs[std::string("a")] = 3;
        hs[std::string("b")] = 4;
        printf("size-%lu\n", hs.size());
        BOOST_CHECK_EQUAL(hs.size(), 2);
    }
    hs.clear();
    printf("TestHax done...\n");
}

BOOST_AUTO_TEST_CASE(FilePart) {
    {
        emulex::fs::SortedPart fp(100);
        fp.add(10, 20);
        fp.add(21, 30);
        fp.add(31, 40);
        BOOST_CHECK_EQUAL(fp.size(), 2);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(200, 300);
        BOOST_CHECK_EQUAL(fp.add(1000, 1100), false);
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 300);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(500, 600);
        BOOST_CHECK_EQUAL(fp.size(), 8);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(200, 300);
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 400);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(150, 300);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 400);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(150, 400);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 400);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(150, 500);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 500);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(10, 90);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 8);
        BOOST_CHECK_EQUAL(fp[0], 10);
        BOOST_CHECK_EQUAL(fp[1], 90);
        BOOST_CHECK_EQUAL(fp[2], 100);
        BOOST_CHECK_EQUAL(fp[3], 200);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(110, 190);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 6);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 200);
        BOOST_CHECK_EQUAL(fp[2], 300);
        BOOST_CHECK_EQUAL(fp[3], 400);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(110, 320);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 4);
        BOOST_CHECK_EQUAL(fp[0], 100);
        BOOST_CHECK_EQUAL(fp[1], 400);
        BOOST_CHECK_EQUAL(fp[2], 1000);
        BOOST_CHECK_EQUAL(fp[3], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(0, 100);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 6);
        BOOST_CHECK_EQUAL(fp[0], 0);
        BOOST_CHECK_EQUAL(fp[1], 200);
        BOOST_CHECK_EQUAL(fp[2], 300);
        BOOST_CHECK_EQUAL(fp[3], 400);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.print();
        fp.add(300, 400);
        fp.print();
        fp.add(1000, 1100);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(0, 1100);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 2);
        BOOST_CHECK_EQUAL(fp[0], 0);
        BOOST_CHECK_EQUAL(fp[1], 1100);
    }
    {
        emulex::fs::SortedPart fp(1101);
        fp.add(100, 200);
        fp.add(300, 400);
        fp.add(1000, 1100);
        BOOST_CHECK_EQUAL(fp.size(), 6);
        fp.add(0, 1200);
        fp.print();
        BOOST_CHECK_EQUAL(fp.size(), 2);
        BOOST_CHECK_EQUAL(fp[0], 0);
        BOOST_CHECK_EQUAL(fp[1], 1200);
    }
    {
        emulex::fs::SortedPart fp(100);
        BOOST_CHECK_EQUAL(fp.add(0, 10), false);
        BOOST_CHECK_EQUAL(fp.add(50, 99), false);
        BOOST_CHECK_EQUAL(fp.add(10, 50), true);
    }
    printf("FilePart done...\n");
}

BOOST_AUTO_TEST_CASE(PartStatus) {
    emulex::fs::SortedPart fp(12 * ED2K_PART_L + 100);
    fp.add(100, 200);
    fp.add(ED2K_PART_L, 2 * ED2K_PART_L - 1);
    fp.add(3 * ED2K_PART_L, 4 * ED2K_PART_L - 100);
    fp.add(5 * ED2K_PART_L, 7 * ED2K_PART_L - 1);
    fp.add(9 * ED2K_PART_L, 11 * ED2K_PART_L - 1);
    auto ps = fp.parsePartStatus();
    BOOST_CHECK_EQUAL(ps.size(), 2);
    uint8_t v = ps[0];
    BOOST_CHECK_EQUAL((v >> 7) & 1, 0);
    BOOST_CHECK_EQUAL((v >> 6) & 1, 1);
    BOOST_CHECK_EQUAL((v >> 5) & 1, 0);
    BOOST_CHECK_EQUAL((v >> 4) & 1, 0);
    BOOST_CHECK_EQUAL((v >> 3) & 1, 0);
    BOOST_CHECK_EQUAL((v >> 2) & 1, 1);
    BOOST_CHECK_EQUAL((v >> 1) & 1, 1);
    BOOST_CHECK_EQUAL((v >> 1) & 0, 0);
    uint8_t v2 = ps[1];
    BOOST_CHECK_EQUAL((v2 >> 7) & 1, 0);
    BOOST_CHECK_EQUAL((v2 >> 6) & 1, 1);
    BOOST_CHECK_EQUAL((v2 >> 5) & 1, 1);
    BOOST_CHECK_EQUAL((v2 >> 4) & 1, 0);
    BOOST_CHECK_EQUAL((v2 >> 3) & 1, 0);
    BOOST_CHECK_EQUAL((v2 >> 2) & 1, 0);
    BOOST_CHECK_EQUAL((v2 >> 1) & 1, 0);
    BOOST_CHECK_EQUAL((v2 >> 0) & 1, 0);
    //    BOOST_CHECK_EQUAL(fp[0], 0);
    //    BOOST_CHECK_EQUAL(fp[1], 1200);
    printf("PartStatus done...\n");
}

BOOST_AUTO_TEST_CASE(FileConf) {
    boost::filesystem::remove_all(boost::filesystem::path("xx.xcf"));
    emulex::fs::FileConf fc = emulex::fs::BuildFileConf(100);
    fc->name = BuildData("testing", 7, true);
    fc->size = 100;
    fc->md4 = BuildHash(16);
    fc->ed2k.push_back(BuildHash(16));
    fc->ed2k.push_back(BuildHash(16));
    fc->md5 = BuildHash(20);
    fc->sha1 = BuildHash(20);
    fc->parts.add(100, 300);
    fc->save("xx.xcf");
    emulex::fs::FileConf fc2 = emulex::fs::BuildFileConf(100);
    fc2->read("xx.xcf");
    fc->name->print();
    fc2->name->print();
    BOOST_CHECK_EQUAL(strcmp(fc->name->data, fc2->name->data), 0);
    BOOST_CHECK_EQUAL(fc->size, fc2->size);
    BOOST_CHECK_EQUAL(fc2->ed2k.size(), 2);
    BOOST_CHECK_EQUAL(fc2->parts.size(), 2);
    //
    emulex::encoding::Encoding enc;
    enc.put((uint8_t)0x40).put((uint16_t)0);
    enc.put((uint8_t)0x70).put((uint16_t)0);
    enc.put((uint8_t)0x00).put((uint16_t)0);
    std::fstream file;
    file.open("xx2.xcf", std::fstream::out | std::fstream::trunc);
    file.write(enc.cbuf(), enc.size());
    file.close();
    fc2->read("xx2.xcf");
    BOOST_CHECK_EQUAL(fc2->ed2k.size(), 0);
    BOOST_CHECK_EQUAL(fc2->parts.size(), 0);
    printf("FileConf done...\n");
}

BOOST_AUTO_TEST_CASE(ReadHash) {
    boost::filesystem::remove_all(boost::filesystem::path("rh.dat"));
    std::fstream fs;
    fs.open("rh.dat", std::fstream::out);
    fs << "abcd";
    fs.close();
    auto pp = boost::filesystem::path("/tmp");
    std::cout << pp << std::endl;
    auto bb = pp.append("abc");
    std::cout << pp << std::endl;
    std::cout << bb << std::endl;
    emulex::fs::FileConf fc = emulex::fs::BuildFileConf(100);
    BOOST_CHECK_EQUAL(fc->readhash("rh.dat", true, true, true), 0);
    BOOST_CHECK_EQUAL(strcmp(fc->name->data, "rh.dat"), 0);
    BOOST_CHECK_EQUAL(fc->size, 4);
    BOOST_CHECK_EQUAL(fc->ed2k.size(), 1);
    BOOST_CHECK_EQUAL(fc->md4->len, 16);
    fc->md4->print();
    fc->md5->print();
    fc->sha1->print();
    printf("ReadHash done...\n");
}

BOOST_AUTO_TEST_CASE(Task) {
    boost::filesystem::remove_all(boost::filesystem::path("task.db"));
    auto tdb = FTaskDb(new FTaskDb_);
    tdb->init("task.db");
    FTask task = FTask(new FTask_);
    task->filename = BuildData("abc.txt", 7, true);
    task->location = BuildData("abc.txt", 7, true);
    task->size = 7;
    task->format = BuildData(".txt", 4);
    task->status = FTSS_RUNNING;
    auto tid = tdb->add(task);
    BOOST_CHECK_EQUAL(tid, 1);
    BOOST_CHECK_EQUAL(tdb->count(), 1);
    auto ts = tdb->list();
    BOOST_CHECK_EQUAL(ts.size(), 1);
    tdb->remove(tid);
    BOOST_CHECK_EQUAL(tdb->count(), 0);
    printf("Task done...\n");
}

BOOST_AUTO_TEST_CASE(Data) {
    boost::filesystem::remove_all(boost::filesystem::path("data.db"));
    auto tdb = FDataDb(new FDataDb_);
    tdb->init("data.db");
    FData data = FData(new FData_);
    data->filename = BuildData("abc.txt", 7, true);
    data->location = BuildData("abc.txt", 7, true);
    data->size = 7;
    data->format = BuildData(".txt", 4);
    data->status = FTSS_RUNNING;
    auto tid = tdb->add(task);
    BOOST_CHECK_EQUAL(tid, 1);
    BOOST_CHECK_EQUAL(tdb->count(), 1);
    auto ts = tdb->list();
    BOOST_CHECK_EQUAL(ts.size(), 1);
    tdb->remove(tid);
    BOOST_CHECK_EQUAL(tdb->count(), 0);
    printf("Task done...\n");
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* fs_test_h */
