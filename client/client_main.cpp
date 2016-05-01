//
// Created by yzhh on 4/10/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "RaftFS.h"
#include "../utils/Options.h"
#include "PerfTest.h"



using namespace std;
using namespace raftfs;
using namespace raftfs::client;

#define TEST_MODULE_ENABLE	1

int main(int argc, char** argv) {

    Options opt(argc, argv);
    cout << "tt" << endl;
    FSClient client(opt);

    //client.CheckLeaders();
#if(TEST_MODULE_ENABLE==0)
    string path("/test");
    for (int i = 0; i < 3; ++i) {
        string p = path + to_string(i);
        cout << "mkdir " << p << " " << client.Mkdir(p) << endl;
    }
    this_thread::sleep_for(chrono::seconds(1));
    vector<string> list;
    cout << " list root " << client.ListDir("/", list) << endl;
    for (auto& l: list) {
        cout << l << endl;
    }
/*
    client.Mkdir("/read");
    client.CreateFile("/read/file1");
    client.CreateFile("/read/file2");
    client.CreateFile("/read/file3");


    string file("/test0/file1");
    cout << "creat " << file << " " << client.CreateFile(file) << endl;*/

#endif
    cout << "hello world" << endl;


#if(TEST_MODULE_ENABLE == 1)
#define TOTAL_CMDS	100
typedef PerfTest::perf_cmd_type pcmd_type;

    PerfTest::PerfTestParameters para;
    para.filename = "perf_test_1.log";
    para.max_cmds = 10;
    // Read / Write
    para.cmd_ratio[pcmd_type::perf_mkdir] = 0;
    para.cmd_ratio[pcmd_type::perf_createfile] = 0;
    para.cmd_ratio[pcmd_type::perf_delete] = 0;
    // Read Only
    para.cmd_ratio[pcmd_type::perf_listdir] = 25;
    para.cmd_ratio[pcmd_type::perf_getfinfo] = 25;
    PerfTest ptest1(&client, &para);

    ptest1.create_test_tree(2);
    ptest1.result_write_head();
    //ptest1.run();
#endif

    return 0;
}
