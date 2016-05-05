/*
 * PerfTest.h
 *  Created on: Apr 28, 2016
 *      Author: huang630
 */
#ifndef PERF_TEST_H
#define PERF_TEST_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <boost/shared_ptr.hpp>
#include <transport/TSocket.h>

#include "../utils/Options.h"

// sure that we are using.
#include "../protocol/ClientService.h"		// Take advantage of client_types.h
#include <fstream>
#include "RaftFS.h"
using namespace std;

namespace THt = apache::thrift::transport;


namespace raftfs {
    namespace client {

        typedef protocol::Status::type Status_;
        typedef chrono::system_clock::time_point Tp;

        class PerfTest {
        public:
            //---------------------------------------
            // Test record structures
        	enum perf_cmd_type {
                perf_mkdir = 0,
                perf_rmdir,
                perf_listdir,
                perf_getfinfo,
                perf_createfile,
                perf_delete,
                perf_cmd_max
            };

        	typedef struct test_para {
            	string filename;
            	int cmd_ratio[perf_cmd_max];
            	int64_t	max_cmds;
            } PerfTestParameters;

            typedef struct test_rec {
            	int cmd; 	string node;	int err_no;
            } PerfTestRec;


            //---------------------------------------
            // Test Tree Structures
            typedef struct test_dir_node {
            	struct test_dir_node * parent;	// nullptr if root
            	string fullname;
            	bool exists;
                bool should_exist;
            	test_dir_node(test_dir_node * ptr, string name) {
            		parent = ptr;	fullname = name;
            		exists = false;
                    should_exist = false;
            	}
            } PerfTestNode;

            //---------------------------------------
            // Functions
            PerfTest(FSClient * test_through_client, PerfTest::PerfTestParameters * paras);
            ~PerfTest();
            void run();		// main entry

            void create_test_tree(int test_case, bool do_rpc);
            void result_write_head();
            void result_write_line(PerfTestRec * result);
            void result_write_all_records();
            void result_write_stats();	// TODO: finish this.


            chrono::system_clock::duration total_read_latency; // in us
            int64_t read_count = 0;
            chrono::system_clock::duration total_write_latency; // in us
            int64_t write_count = 0;
            chrono::system_clock::duration correct_write_latency; // in us
            int64_t correct_write_count = 0;

        private:
            FSClient* client;
            ofstream result_file;
            std::vector<PerfTestRec *> records;
            //-------------------------------------
            // Test Running datas
            int cmd_ratio[perf_cmd_max];			// theory value
            int cmd_limit[perf_cmd_max];
            int cmd_count[perf_cmd_max];
            int cmd_cumulated_gate[perf_cmd_max];	// for runtime execution.
            int64_t	cmd_total_to_run;
            int64_t	cmd_executed;
            //-------------------------------------
            // Test Tree structure.
            std::vector<PerfTestNode *> test_tree;

            std::vector<PerfTestNode *> read_tree;
            std::vector<PerfTestNode *> write_tree;

			// stat



        };
    }
}

#endif // PERF_TEST_H
