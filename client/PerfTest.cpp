/*
 * PerfTest.cpp
 *  Created on: Apr 28, 2016
 *      Author: huang630
 */

#include "PerfTest.h"
#include <vector>
#include <fstream>
#include <iomanip>      // std::put_time
#include <ctime>        // std::time_t, struct std::tm, std::localtime
#include <chrono>       // std::chrono::system_clock
#include "RaftFS.h"
#include "../utils/time_utils.h"

using namespace std::chrono;
using namespace raftfs::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift;
using namespace apache::thrift::protocol;



using namespace std;


namespace raftfs {
    namespace client {
        PerfTest::PerfTest(FSClient *test_through_client,
                           PerfTest::PerfTestParameters *paras)
                : cmd_executed(0), client(nullptr)
        //follower_id(rand() % hosts.size() + 1)
        {
            assert(test_through_client != nullptr);        // must be valid.
            client = test_through_client;

            // Open result file.
            result_file.open(paras->filename, std::fstream::out);
            assert(result_file.is_open());        // file can not be opened.

            // Copy cmd_ratios
            int acc = 0;
            for (int i = 0; i < perf_cmd_max; ++i) {
                cmd_ratio[i] = paras->cmd_ratio[i];
                cmd_cumulated_gate[i] = acc + paras->cmd_ratio[i];
                acc += paras->cmd_ratio[i];
            }
            assert(acc <= 100);        // accumulated cmd ratio should be 100!
            // Maximum Filesystem / Metadata commands to be executed.
            this->cmd_total_to_run = paras->max_cmds;

            // Other init.
            std::srand(std::time(0));
        }

        PerfTest::~PerfTest() {
            // TODO: if record file is not close. Close it.
            if (result_file.is_open())
                result_file.close();

            for (auto p: test_tree) {
                delete p;
            }

            for (auto p: read_tree) {
                delete p;
            }

            for (auto p: write_tree) {
                delete p;
            }
        }

        void PerfTest::create_test_tree(int test_case) {
            switch (test_case) {
                case 1: {
                    PerfTestNode *dir1 = new PerfTestNode(nullptr, "/dir1");
                    test_tree.push_back(dir1);
                    PerfTestNode *file11 = new PerfTestNode(dir1, "/dir1/file11");
                    PerfTestNode *file12 = new PerfTestNode(dir1, "/dir1/file12");
                    PerfTestNode *file13 = new PerfTestNode(dir1, "/dir1/file13");
                    test_tree.push_back(file11);
                    test_tree.push_back(file12);
                    test_tree.push_back(file13);
                    //
                    PerfTestNode *dir2 = new PerfTestNode(nullptr, "/dir2");
                    test_tree.push_back(dir2);
                    PerfTestNode *dir21 = new PerfTestNode(dir2, "/dir1/dir21");
                    PerfTestNode *dir22 = new PerfTestNode(dir2, "/dir1/dir22");
                    PerfTestNode *dir23 = new PerfTestNode(dir2, "/dir1/dir23");
                    test_tree.push_back(dir21);
                    test_tree.push_back(dir22);
                    test_tree.push_back(dir23);
                    //
                    PerfTestNode *dir3 = new PerfTestNode(nullptr, "/dir2");
                    test_tree.push_back(dir3);
                    //PerfTestNode* dir4 = new PerfTestNode(dir2, "/dir2");

                    break;
                }
                case 2: {

                    cout << "initializing test case 2" << endl;

                    PerfTestNode *dir1 = new PerfTestNode(nullptr, "/read");
                    assert(Status_::kOK == client->Mkdir("/read"));
                    dir1->should_exist = true;
                    PerfTestNode *file11 = new PerfTestNode(dir1, "/read/file11");
                    assert(Status_::kOK == client->CreateFile("/read/file11"));
                    file11->should_exist = true;
                    PerfTestNode *file12 = new PerfTestNode(dir1, "/read/file12");
                    assert(Status_::kOK == client->CreateFile("/read/file12"));
                    file12->should_exist = true;
                    PerfTestNode *file13 = new PerfTestNode(dir1, "/read/file13");
                    assert(Status_::kOK == client->CreateFile("/read/file13"));
                    file13->should_exist = true;

                    vector<string> list;
                    client->ListDir("/read", list);
                    for (auto &l: list) {
                        cout << l << endl;
                    }

                    read_tree.push_back(dir1);
                    read_tree.push_back(file11);
                    read_tree.push_back(file12);
                    read_tree.push_back(file13);

                    PerfTestNode *dir2 = new PerfTestNode(nullptr, "/write");
                    assert(Status_::kOK == client->Mkdir("/write"));
                    dir2->should_exist = true;
                    write_tree.push_back(dir2);

                    PerfTestNode *file21 = new PerfTestNode(dir1, "/write/file21");
                    PerfTestNode *file22 = new PerfTestNode(dir1, "/write/file22");
                    PerfTestNode *file23 = new PerfTestNode(dir1, "/write/file23");

                    write_tree.push_back(file21);
                    write_tree.push_back(file22);
                    write_tree.push_back(file23);

                    PerfTestNode *dir21 = new PerfTestNode(dir2, "/dir1/dir21");
                    PerfTestNode *dir22 = new PerfTestNode(dir2, "/dir1/dir22");
                    PerfTestNode *dir23 = new PerfTestNode(dir2, "/dir1/dir23");
                    test_tree.push_back(dir21);
                    test_tree.push_back(dir22);
                    test_tree.push_back(dir23);

                    cout << "case 2 initialized" << endl;
                    break;
                }

                default:
                    cout << "ERROR: test case " << test_case
                    << "does not exists!" << endl;
                    break;
            }
        }

        //-------------------------------------------------------
        // Test Main Entry
        //-------------------------------------------------------
        void PerfTest::run() {
            // TODO: run perf test flow...
            int next_cmd, tmp;
            std::vector<std::string> tmp_dir_list;
            FileInfo tmp_file_info;
            int test_node_index = 0;

            cout << "Perf Test Begin: " << endl;

            // TODO: count total time since all results now are pushed into mem
            while (cmd_executed < cmd_total_to_run) {

                //----------------------------------------------
                // Decide which cmd to be exec.
                int tmp_count;
                do {
                    next_cmd = rand() % perf_cmd_max;
                    if (next_cmd == perf_listdir || next_cmd == perf_getfinfo) {
                        tmp_count = read_count + 1;
                    } else {
                        tmp_count = write_count + 1;
                    }

                } while ((tmp_count / static_cast<double>(cmd_total_to_run)) > (this->cmd_ratio[next_cmd]/ 100.0));
                /*
                tmp = std::rand() % 100;
                for (int i = 0; i < perf_cmd_max; ++i) {
                    if (tmp < this->cmd_cumulated_gate[i]) {
                        next_cmd = i;
                        break;
                    }
                }
                 */

                PerfTestRec *rec_result = new PerfTestRec();

                PerfTestNode* tmp_node = nullptr;
                int i;
                switch (next_cmd) {
                    case perf_mkdir:
                        i = 4;
                        do {
                            tmp_node = write_tree[i];
                            ++i;
                        } while (i < write_tree.size() && tmp_node->should_exist);
                        if (i == write_tree.size())
                            continue;
                        break;
                    case perf_createfile:
                        i = 1;
                        do {
                            tmp_node = write_tree[i];
                            ++i;
                        } while (i < 4 && tmp_node->should_exist);
                        if (i == 4)
                            continue;
                        break;
                    case perf_delete:
                        i = 1;
                        do {
                            tmp_node = write_tree[i];
                            ++i;
                        } while (i < write_tree.size() && !tmp_node->should_exist);
                        if (i == write_tree.size())
                            continue;
                        break;
                    case perf_getfinfo:
                        tmp_node = read_tree[1];
                        break;
                    case perf_listdir:
                        tmp_node = read_tree[0];
                        break;
                    default:
                        ;
                }
                /*
#if(1)

                PerfTestNode *tmp_node = test_tree[test_node_index % test_tree.size()];
#else
                PerfTestNode tmp_node(nullptr, "/dir1");		// TODO: get from array / vector
#endif
                */


                //Status_ rtn = Status_::kOK;
                int rtn = Status_::kOK;
                //----------------------------------------------
                // Run commands
                // Return status:
                /*kOK = 0,
                kNoLeader = 1,
                kNotFound = 2,
                kTimeout = 3,
                kExist = 4,
                kCommError = 5
                */
                /*
                cout << "cmd: " << next_cmd << " name: " <<
                        tmp_node.fullname << " client " << client << endl;
                client->Mkdir(tmp_node.fullname);
                cout << "cmd: " << next_cmd << " client " << client << endl;
                */
                // FIXME: Assume we send to leader first via FSClient interface.
                switch (next_cmd) {
                    case perf_mkdir: {

                        Tp start = Now();
                        rtn = client->Mkdir(tmp_node->fullname);
                        total_write_latency += Now() - start;
                        ++write_count;
                        // Record dir exists or not
                        if (rtn == Status_::kOK) {

                            tmp_node->should_exist = true;
                            /*if (tmp_node->exists) {
                                cout << *//*result_file <<*//* "ERROR: mkdir " << tmp_node->fullname << endl;
                                rtn = -1;
                            } else {
                                tmp_node->exists = true;    // since we make this dir.
                            }*/
                        }
                        break;
                    }
                    case perf_listdir: {
                        Tp start = Now();
                        rtn = client->ListDir(tmp_node->fullname, tmp_dir_list);
                        total_read_latency += Now() - start;
                        ++read_count;
                        // Record dir exists or not
                        if (rtn == Status_::kOK) {
                            if (!tmp_node->exists) {
                                cout << /*result_file <<*/ "ERROR: listdir " << tmp_node->fullname << endl;
                                rtn = -1;
                            } else {
                                // works fine.
                            }
                        }
                        break;
                    }
                    case perf_getfinfo: {
                        Tp start = Now();

                        rtn = client->GetFileInfo(tmp_node->fullname, tmp_file_info);
                        total_read_latency += Now() - start;
                        ++read_count;
                        // Record dir exists or not
                        if (rtn == Status_::kOK) {
                            if (!tmp_node->exists) {
                                cout << /*result_file <<*/ "ERROR: getinfo " << tmp_node->fullname << endl;
                                rtn = -1;
                            } else {
                                // works fine.
                            }
                        }
                        break;
                    }
                    case perf_createfile: {
                        Tp start = Now();

                        rtn = client->CreateFile(tmp_node->fullname);
                        total_write_latency += Now() - start;
                        ++write_count;


                        // Record dir exists or not
                        if (rtn == Status_::kOK) {
                            tmp_node->should_exist = true;
                            /*if (tmp_node->exists) {
                                cout << *//**//*result_file <<*//**//* "ERROR: createfile " << tmp_node->fullname << endl;
                                rtn = -1;
                            } else {
                                tmp_node->exists = true;    // since we make this file.
                            }*/
                        }
                        break;
                    }
                    case perf_delete: {
                        Tp start = Now();

                        rtn = client->Delete(tmp_node->fullname);
                        total_write_latency += Now() - start;
                        ++write_count;
                        // Record dir exists or not
                        // TODO: check behavior when delete dir / file not exists...
                        if (rtn == Status_::kOK) {
                            tmp_node->should_exist = false;
                        }
                            /*if (!tmp_node->exists) {
                                cout << *//*result_file <<*//* "ERROR: delete " << tmp_node->fullname << endl;
                                rtn = -1;
                            } else {
                                tmp_node->exists = false;    // since we delete this dir/file
                            }*/

                        break;
                    }
                    default:
                        cout << "ERROR: Wrong command..." << endl;
                        break;
                }

                //-- Result output and recording
                rec_result->cmd = next_cmd;
                rec_result->err_no = rtn;
                rec_result->node = tmp_node->fullname;
                records.push_back(rec_result);

                cout << cmd_executed << "/" << cmd_total_to_run << endl;
                // Preparation for next round.
                cmd_executed++;
                test_node_index++;
            }

            cout << "test finished" << endl;
        }

        void PerfTest::result_write_head() {
            //cout << "write file test" << endl;

            std::chrono::system_clock::time_point p = system_clock::now();
            std::time_t t = system_clock::to_time_t(p);
            result_file << "Test on: " << std::ctime(&t) << endl;
        }

        void PerfTest::result_write_line(PerfTestRec *result) {
            result_file << TimePointStr(Now()) << ", "
            << result->cmd << ", "    // TODO: change to cmd string
            << result->node << ", "
            << result->err_no << endl;
        }

        void PerfTest::result_write_all_records() {
            for (auto p: records) {
                result_file << p->cmd << ", "    // TODO: change to cmd string
                << p->node << ", "
                << p->err_no << endl;
            }
        }

        void PerfTest::result_write_stats() {
            int64_t num_readonly = 0, num_readwrite = 0,
                    num_mkdir = 0, num_listdir = 0, num_getinfo = 0,
                    num_create_file = 0, num_delete = 0;
            int64_t cmd_correct = 0;

            // TODO: write time and stats of this test.

            for (auto p: records) {
                if (p->err_no == 0) cmd_correct++;

                switch (p->cmd) {
                    case perf_mkdir:
                        num_mkdir++;
                        num_readwrite++;
                        break;

                    case perf_listdir:
                        num_listdir++;
                        num_readonly++;
                        break;

                    case perf_getfinfo:
                        num_getinfo++;
                        num_readonly++;
                        break;

                    case perf_createfile:
                        num_create_file++;
                        num_readwrite++;
                        break;

                    case perf_delete:
                        num_delete++;
                        num_readwrite++;
                        break;

                    default:
                        cout << "ERROR: Wrong command..." << endl;
                        break;
                }
            }
            result_file << "------------------------" << endl;
            //result_file << "total cmds: " << records.size() << endl;
            result_file << "total cmds: " << read_count + write_count << endl;
            result_file << "avg latency: " << duration_cast<milliseconds>(total_read_latency + total_write_latency).count()
                    / static_cast<double>(read_count + write_count) << endl;
            result_file << "correct cmds: " << cmd_correct << endl;
            result_file << "total read cmds: " << read_count << endl;
            result_file << "avg read latency: " << duration_cast<milliseconds>(total_read_latency).count()
                                              / static_cast<double>(read_count) << endl;
            result_file << "total write cmds: " << write_count << endl;
            result_file << "read/write ratio: " << read_count/ static_cast<double>(write_count) << endl;
            result_file << "avg write latency: " << duration_cast<milliseconds>(total_write_latency).count()
                    / static_cast<double>(write_count);

            //result_file << "Read/Wrrite: " << num_readwrite << " ; ReadOnly: "
            //<< num_readonly << endl;
            result_file << "\t mkdir: " << num_mkdir << endl;
            result_file << "\t createfile: " << num_create_file << endl;
            result_file << "\t delete: " << num_delete << endl;
            result_file << "\t listdir: " << num_listdir << endl;
            result_file << "\t getinfor: " << num_getinfo << endl;

        }

#if(0)    // Functions in FSClient that we may need to overload...
        bool PerfTest::ConnectFollower() {
        int32_t PerfTest::GetLeader() {
        void PerfTest::CheckLeaders() {
        Status_ PerfTest::Mkdir(const string &abs_dir) {
        Status_ PerfTest::ListDir(const std::string &abs_dir, std::vector<std::string> &dir_list) {
        Status_ PerfTest::GetFileInfo(const std::string &file_path, protocol::FileInfo &file_info) {
        Status_ PerfTest::CreateFile(const std::string &file_path) {
        Status_ PerfTest::Delete(const std::string &path) {
        // sock related.
        void PerfTest::ResetSock(boost::shared_ptr<THt::TSocket> &sock, string host) {
        bool PerfTest::ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client,
                                          boost::shared_ptr<THt::TSocket> &sock,
                                          string host) {

#endif


    }    // end of namespace client
}    // end of namespace raftfs
