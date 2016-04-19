//
// Created by yzhh on 3/29/16.
// - GetLeader: For a client to get current leader.
//

#include "ClientRPCService.h"

#include <iostream>

using namespace std;
using namespace raftfs::protocol;

namespace raftfs {
    namespace server {

        void ClientRPCService::GetLeader(GetLeaderResponse &_return) {
            _return.leader_id = raft_state.GetLeader();
            cout << " recv GET LEADER " << _return.leader_id << endl;
        }


        void ClientRPCService::Mkdir(MkdirResponse &_return, const MkdirRequest &new_dir) {
            cout << " recv MKDIR " << new_dir.path << endl;
            raft_state.OnMetaOperation(MetaOp::kMkdir, new_dir.path, nullptr);
        }

        void ClientRPCService::InjectTestCase(protocol::TestCaseResponse& _return, const protocol::TestCaseRequest& req) {
            // TODO: call test case functions.
        	cout << " Inject test case: " << req.test_type << endl;
            //raft_state.OnMetaOperation(MetaOp::kMkdir, new_dir.path, nullptr);
        }
    }
}
