/*
 * PerfTest.cpp
 *  Created on: Apr 28, 2016
 *      Author: huang630
 */

#include "PerfTest.h"
#include <vector>
#include <chrono>
#include "RaftFS.h"
#include <fstream>

using namespace raftfs::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift;
using namespace apache::thrift::protocol;



using namespace std;


namespace raftfs {
    namespace client {
        PerfTest::PerfTest(FSClient * test_through_client,
        			PerfTest::PerfTestParameters * paras)
                : cmd_executed(0)
                  //follower_id(rand() % hosts.size() + 1)
        {
        	assert(test_through_client == nullptr);		// must be valid.

        	// Open result file.
        	result_file.open(paras->filename, std::fstream::out);
        	assert(!result_file.is_open());		// file can not be opened.

        	// Copy cmd_ratios
        	int acc = 0;
        	for(int i=0; i<perf_cmd_max; ++i) {
        		cmd_ratio[i] = paras->cmd_ratio[i];
        		cmd_cumulated_gate[i] = acc + paras->cmd_ratio[i];
        		acc += paras->cmd_ratio[i];
        	}
        	assert(acc != 100);		// accumulated cmd ratio should be 100!
        	// Maximum Filesystem / Metadata commands to be executed.
        	this->cmd_total_to_run = paras->max_cmds;

            // Other init.
            std::srand(std::time(0));
        }

        PerfTest::~PerfTest() {
            // TODO: if record file is not close. Close it.
        	if(result_file.is_open())
        		result_file.close();
        }

        //-------------------------------------------------------
        // Test Main Entry
        //-------------------------------------------------------
        void PerfTest::run() {
        	// TODO: run perf test flow...
        	int next_cmd, tmp;
        	std::vector<std::string> tmp_dir_list;
        	FileInfo tmp_file_info;

        	while(cmd_executed < cmd_total_to_run) {
        		//----------------------------------------------
        		// Decide which cmd to be exec.
        		tmp = std::rand() % 100;
        		for(int i=0; i<perf_cmd_max; ++i) {
        			if(tmp < this->cmd_cumulated_gate[i]) {
        				next_cmd = i;
        				break;
        			}
        		}

        		PerfTestNode tmp_node(nullptr, "/dir1");		// TODO: get from array / vector
        		Status_ rtn = Status_::kOK;
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
        		// FIXME: Assume we send to leader first via FSClient interface.
        		switch(next_cmd) {
        		case perf_mkdir:
        			rtn = client->Mkdir(tmp_node.fullname);
        			break;

        		case perf_listdir:
        			rtn = client->ListDir(tmp_node.fullname, tmp_dir_list);
        			break;

        		case perf_getfinfo:
        			rtn = client->GetFileInfo(tmp_node.fullname, tmp_file_info);
        			break;

        		case perf_createfile:
        			rtn = client->CreateFile(tmp_node.fullname);
        			break;

        		case perf_delete:
        			rtn = client->Delete(tmp_node.fullname);
        			break;

        		default:
        			cout << "ERROR: Wrong command..." << endl;
                	break;
        		}


        		cmd_executed++;
        	}
        }

        void PerfTest::result_write_head() {
        	result_file << "Hello World";
        }

#if(0)	// Functions in FSClient that we may need to overload...
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



    }	// end of namespace client
}	// end of namespace raftfs
