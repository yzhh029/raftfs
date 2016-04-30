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

#define TEST_MODULE_ENABLE	0

int main(int argc, char** argv) {

    Options opt(argc, argv);
    cout << "tt" << endl;
    FSClient client(opt);

    //client.CheckLeaders();

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
    cout << "hello world" << endl;


#if(TEST_MODULE_ENABLE == 1)
#define TOTAL_CMDS	100
typedef PerfTest::perf_cmd_type pcmd_type;

    PerfTest::PerfTestParameters para;
    para.filename = "perf_test_1.log";
    para.max_cmds = 3;
    // Read / Write
    para.cmd_ratio[pcmd_type::perf_mkdir] = 18;
    para.cmd_ratio[pcmd_type::perf_createfile] = 18;
    para.cmd_ratio[pcmd_type::perf_delete] = 14;
    // Read Only
    para.cmd_ratio[pcmd_type::perf_listdir] = 25;
    para.cmd_ratio[pcmd_type::perf_getfinfo] = 25;
    PerfTest ptest1(&client, &para);
    ptest1.result_write_head();
    ptest1.run();
#endif

    return 0;
}
