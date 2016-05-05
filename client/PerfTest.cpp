/*
 * PerfTest.cpp
 *  Created on: Apr 28, 2016
 *      Author: huang630
 */

#include "PerfTest.h"
#include <vector>
#include <fstream>
#include <cmath>
#include <functional>
#include <numeric>
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
            result_file.open(paras->filename, std::fstream::out | fstream::trunc);
            assert(result_file.is_open());        // file can not be opened.

            // Maximum Filesystem / Metadata commands to be executed.
            this->cmd_total_to_run = paras->max_cmds;

            // Copy cmd_ratios
            int acc = 0;
            for (int i = 0; i < perf_cmd_max; ++i) {
                /*cmd_ratio[i] = paras->cmd_ratio[i];
                cmd_cumulated_gate[i] = acc + paras->cmd_ratio[i];
                acc += paras->cmd_ratio[i];*/
                cmd_count[i] = 0;
                cmd_limit[i] = cmd_total_to_run * (paras->cmd_ratio[i] / (100.0));
            }
            int sum = std::accumulate(begin(cmd_limit), end(cmd_limit), 0, std::plus<int>());        // accumulated cmd ratio should be 100!
            assert(sum <= cmd_total_to_run);

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

        void PerfTest::create_test_tree(int test_case, bool do_rpc) {
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
                    dir1->should_exist = true;
                    PerfTestNode *file11 = new PerfTestNode(dir1, "/read/file11");
                    file11->should_exist = true;
                    PerfTestNode *file12 = new PerfTestNode(dir1, "/read/file12");
                    file12->should_exist = true;
                    PerfTestNode *file13 = new PerfTestNode(dir1, "/read/file13");
                    file13->should_exist = true;

                    if (do_rpc) {
                        assert(Status_::kOK == client->Mkdir("/read"));
                        assert(Status_::kOK == client->CreateFile("/read/file11"));
                        assert(Status_::kOK == client->CreateFile("/read/file12"));
                        assert(Status_::kOK == client->CreateFile("/read/file13"));
                    }


                    vector<string> list;
                    client->ListDir("/read", list);
                    for (auto &l: list) {
                        cout << l << endl;
                    }

                    read_tree.push_back(dir1); //0
                    read_tree.push_back(file11);
                    read_tree.push_back(file12);
                    read_tree.push_back(file13); //3

                    PerfTestNode *dir2 = new PerfTestNode(nullptr, "/write");
                    if (do_rpc)
                    {
                    	int tmp_sta = client->Mkdir("/write");
                    	cout << tmp_sta << endl;
                    	// Have problem here if create...
                    	assert(Status_::kOK == client->Mkdir("/write"));
                    }
                    dir2->should_exist = true;
                    write_tree.push_back(dir2); //0

                    PerfTestNode *file21 = new PerfTestNode(dir1, "/write/file21");
                    PerfTestNode *file22 = new PerfTestNode(dir1, "/write/file22");
                    PerfTestNode *file23 = new PerfTestNode(dir1, "/write/file23");

                    write_tree.push_back(file21); //1
                    write_tree.push_back(file22);
                    write_tree.push_back(file23); //3

                    PerfTestNode *dir21 = new PerfTestNode(dir2, "/write/dir21");
                    PerfTestNode *dir22 = new PerfTestNode(dir2, "/write/dir22");
                    PerfTestNode *dir23 = new PerfTestNode(dir2, "/write/dir23");
                    write_tree.push_back(dir21); // 4
                    write_tree.push_back(dir22);
                    write_tree.push_back(dir23); //6

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
            int no_valid_cmd_cnt = 0;

            cout << "Perf Test Begin: " << endl;

            // TODO: count total time since all results now are pushed into mem
            while (cmd_executed < cmd_total_to_run) {

                //----------------------------------------------
                // Decide which cmd to be exec.
                int tmp_count;
                do {
                    next_cmd = rand() % perf_cmd_max;
                    tmp_count = cmd_count[next_cmd] + 1;
                } while (tmp_count > cmd_limit[next_cmd]);

                if(no_valid_cmd_cnt > 1000) {
                	cout << "No valid commands to run!! (Won't have commands that can return true)" << endl;
                	cout << "Last invalid command: " << next_cmd << endl;
                	cout << "Write tree: " << write_tree.size() << endl;
					#if(0)
                	for(auto p: write_tree) {
                		cout << p->should_exist << " ";
                	}
					#endif
                	cout << "Force the test to stop here!!" << endl;
                	break;
                }

                PerfTestNode* tmp_node = nullptr;

                // set cmd parameter
                int i;
                switch (next_cmd) {
                    case perf_mkdir:	// 0
                        i = 4;
                        do {
                            tmp_node = write_tree[i];
                            //cout << "i= " << i << " " << "exist: " << tmp_node->should_exist << endl;
                            ++i;
                        } while (i < write_tree.size() && tmp_node->should_exist);
                        if (i == write_tree.size()) {
                        	no_valid_cmd_cnt++;
                            continue;
                        }
                        break;
                    case perf_createfile:	// 3
                        i = 1;
                        do {
                            tmp_node = write_tree[i];
                            //cout << "i= " << i << " " << "exist: " << tmp_node->should_exist << endl;
                            ++i;
                        } while (i < 4 && tmp_node->should_exist);
                        if (i == 4) {
                        	no_valid_cmd_cnt++;
                            continue;
                        }
                        break;
                    case perf_delete:	// 4
                        i = 1;
                        do {
                            tmp_node = write_tree[i];
                            //cout << "i= " << i << " " << "exist: " << tmp_node->should_exist << endl;
                            ++i;
                        } while (i < write_tree.size() && !tmp_node->should_exist);
                        if (i == write_tree.size()) {
                        	no_valid_cmd_cnt++;
                            continue;
                        }
                        break;
                    case perf_getfinfo:	// 2
                        tmp_node = read_tree[1];
                        break;
                    case perf_listdir:	// 1
                        tmp_node = read_tree[0];
                        break;
                    default:
                        break;
                }
                no_valid_cmd_cnt = 0;
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

                PerfTestRec *rec_result = new PerfTestRec();	// here for safety.

                //cout << "Enter switch: (after logic check): " << next_cmd << endl;	// debug usage

                // FIXME: Assume we send to leader first via FSClient interface.
                chrono::system_clock::duration tmp_dura;
                switch (next_cmd) {
                    case perf_mkdir: {
                        Tp start = Now();
                        rtn = client->Mkdir(tmp_node->fullname);
                        tmp_dura = Now() - start;
                        //total_write_latency += Now() - start;
                        total_write_latency += tmp_dura;
                        ++write_count;
                        if(rtn == Status_::kOK) {	// acc correct write latency
                        	correct_write_count++;
                        	correct_write_latency += tmp_dura;
                        }
                        // Record dir exists or not
                        cout << "mkdir " << tmp_node->fullname << " done " << endl;	// debug usage
                        tmp_node->should_exist = (rtn == Status_::kOK);

                        break;
                    }
                    case perf_listdir: {
                        Tp start = Now();
                        rtn = client->ListDir(tmp_node->fullname, tmp_dir_list);
                        total_read_latency += Now() - start;
                        ++read_count;
                        cout << "listdir done " << endl;	// debug usagel
                        // Record dir exists or not
                        tmp_node->should_exist = (rtn == Status_::kOK);
                        break;
                    }
                    case perf_getfinfo: {
                        Tp start = Now();

                        rtn = client->GetFileInfo(tmp_node->fullname, tmp_file_info);
                        total_read_latency += Now() - start;
                        ++read_count;
                        // Record dir exists or not
                        tmp_node->should_exist = (rtn == Status_::kOK);
                        break;
                    }
                    case perf_createfile: {
                        Tp start = Now();
                        rtn = client->CreateFile(tmp_node->fullname);
                        //total_write_latency += Now() - start;
                        tmp_dura = Now() - start;
                        total_write_latency += tmp_dura;
                        ++write_count;
                        if(rtn == Status_::kOK) {	// acc correct write latency
                        	correct_write_count++;
                        	correct_write_latency += tmp_dura;
                        }
                        cout << "createfile " << tmp_node->fullname << " done " << endl;	// debug usage
                        tmp_node->should_exist = (rtn == Status_::kOK);

                        break;
                    }
                    case perf_delete: {
                        Tp start = Now();
                        rtn = client->Delete(tmp_node->fullname);
                        //total_write_latency += Now() - start;
                        tmp_dura = Now() - start;
                        total_write_latency += tmp_dura;
                        if(rtn == Status_::kOK) {	// acc correct write latency
                        	correct_write_count++;
                        	correct_write_latency += tmp_dura;
                        }
                        ++write_count;
                        // Record dir exist
                        cout << "delete " << tmp_node->fullname << " done " << endl;	// debug usages or not
                        // TODO: check behavior when delete dir / file not exists...
                        tmp_node->should_exist = !(rtn == Status_::kOK);

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

                ++cmd_count[next_cmd];

                cout << cmd_executed << "/" << cmd_total_to_run << endl;
                // Preparation for next round.
                cmd_executed++;
                test_node_index++;
            }

            cout << "test finished" << endl;
			// Print write tree status to verify server side result manually.
            cout << endl << "Write tree status to verify server status:" << endl;
			for(auto p: write_tree) {
				cout << p->fullname << ": " << p->should_exist << endl;
			}

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
                        break;

                    case perf_listdir:
                        num_listdir++;
                        break;

                    case perf_getfinfo:
                        num_getinfo++;
                        break;

                    case perf_createfile:
                        num_create_file++;
                        break;

                    case perf_delete:
                        num_delete++;
                        break;

                    default:
                        cout << "ERROR: Wrong command..." << endl;
                        break;
                }
            }
            result_file << "------------------------" << endl;
            result_file << "follower: " << client->GetFollowerName() << endl;
            result_file << "total cmds: " << read_count + write_count << endl;
            result_file << "avg latency: " <<
            duration_cast<milliseconds>(total_read_latency + total_write_latency).count()
            / static_cast<double>(read_count + write_count) << endl;
            result_file << "correct cmds: " << cmd_correct << endl;
            if (read_count) {
            result_file << "total read cmds: " << read_count << endl;
            result_file << "avg read latency: " << duration_cast<milliseconds>(total_read_latency).count()
                                                   / static_cast<double>(read_count) << endl;
            }
            if (write_count) {
                result_file << "total write cmds: " << write_count << endl;
                result_file << "avg write latency: " << duration_cast<milliseconds>(total_write_latency).count()
                                                        / static_cast<double>(write_count) << endl;
                result_file << "correct write cmds: " << correct_write_count << endl;
                result_file << "avg correct write latency: " << duration_cast<milliseconds>(correct_write_latency).count()
                                                        / static_cast<double>(correct_write_count) << endl;
            }
            if (read_count && write_count)
                result_file << "read/write ratio: " << read_count / static_cast<double>(write_count) << endl;

            result_file << "\t mkdir: " << num_mkdir << endl;
            result_file << "\t createfile: " << num_create_file << endl;
            result_file << "\t delete: " << num_delete << endl;
            result_file << "\t listdir: " << num_listdir << endl;
            result_file << "\t getinfo: " << num_getinfo << endl;

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
