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

        class PerfTest {
        public:
            //---------------------------------------
            // Test record structures
        	enum perf_cmd_type {
                perf_mkdir = 0,
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
            	test_dir_node(test_dir_node * ptr, string name) {
            		parent = ptr;	fullname = name;
            	}
            } PerfTestNode;

            //---------------------------------------
            // Functions
            PerfTest(FSClient * test_through_client, PerfTest::PerfTestParameters * paras);
            ~PerfTest();
            void run();		// main entry


            void result_write_head();


            /* Things from FSClinet that we may need or not to do tests...
            // only for test purpose
            void CheckLeaders();

            Status_ Mkdir(const std::string &abs_dir);
            Status_ ListDir(const std::string &abs_dir, std::vector<std::string> &dir_list);
            Status_ GetFileInfo(const std::string &file_path, protocol::FileInfo &file_info);
            Status_ CreateFile(const std::string &file_path);
            Status_ Delete(const std::string &path);

            int port;
            int32_t leader_id;
            int32_t follower_id;
            std::vector<std::string> hosts;

            boost::shared_ptr<THt::TSocket> leader_sock;
            boost::shared_ptr<THt::TSocket> follower_sock;
            std::shared_ptr<protocol::ClientServiceClient> leader_rpc;
            std::shared_ptr<protocol::ClientServiceClient> follower_rpc;
            */

        private:
            /*
            int32_t GetLeader();
            bool ConnectFollower();

            void ResetSock(boost::shared_ptr<THt::TSocket> &sock, std::string host);

            bool ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client,
                                boost::shared_ptr<THt::TSocket> &sock,
                                std::string host);
            */

        private:
            FSClient* client;
            fstream result_file;
            //-------------------------------------
            // Test Running datas
            int cmd_ratio[perf_cmd_max];			// theory value
            int cmd_cumulated_gate[perf_cmd_max];	// for runtime execution.
            int64_t	cmd_total_to_run;
            int64_t	cmd_executed;



        };
    }
}

#endif // PERF_TEST_H
