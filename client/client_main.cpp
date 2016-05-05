//
// Created by yzhh on 4/10/16.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include "RaftFS.h"
#include "../utils/Options.h"
#include "PerfTest.h"



using namespace std;
using namespace raftfs;
using namespace raftfs::client;

#define TEST_MODULE_ENABLE	1

mutex m;
bool init = true;

void test_thread(Options* opt, PerfTest::PerfTestParameters param) {

    FSClient client(*opt);
    PerfTest ptest1(&client, &param);

    {
        lock_guard<mutex> lock(m);

        ptest1.create_test_tree(2, init);
        if (init)
            init = false;
    }

    ptest1.result_write_head();
    ptest1.run();
    ptest1.result_write_stats();
}


int main(int argc, char** argv) {

    std::srand(time(0));

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
    para.max_cmds = 100;
#if(0)
    // Read / Write : 50%
    para.cmd_ratio[pcmd_type::perf_mkdir] = 10;
    para.cmd_ratio[pcmd_type::perf_createfile] = 15;
    para.cmd_ratio[pcmd_type::perf_delete] = 25;
    // Read Only : 50%
    para.cmd_ratio[pcmd_type::perf_listdir] = 25;
    para.cmd_ratio[pcmd_type::perf_getfinfo] = 25;
//#elif(1)
    // Read / Write 100
    para.cmd_ratio[pcmd_type::perf_mkdir] = 20;
    para.cmd_ratio[pcmd_type::perf_createfile] = 30;
    para.cmd_ratio[pcmd_type::perf_delete] = 50;
    // Read Only
    para.cmd_ratio[pcmd_type::perf_listdir] = 0;
    para.cmd_ratio[pcmd_type::perf_getfinfo] = 0;
#else
    // Read / Write : 0%
    para.cmd_ratio[pcmd_type::perf_mkdir] = 0;
    para.cmd_ratio[pcmd_type::perf_createfile] = 0;
    para.cmd_ratio[pcmd_type::perf_delete] = 0;
    // Read Only
    para.cmd_ratio[pcmd_type::perf_listdir] = 30;
    para.cmd_ratio[pcmd_type::perf_getfinfo] = 70;
#endif

    /*
    PerfTest ptest1(&client, &para);

    ptest1.create_test_tree(2);
    ptest1.result_write_head();
    ptest1.run();
    ptest1.result_write_stats();*/

    vector<thread> tester;

    int client_num = 4	;

    for (int i = 0; i < client_num; ++i) {
        para.filename = to_string(i) + "perf_test.log";
        tester.push_back(thread(test_thread, &opt, para));
    }

    for (auto& t: tester) {
        t.join();
    }

#endif

    return 0;
}
