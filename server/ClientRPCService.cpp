//
// Created by yzhh on 3/29/16.
//

#include "ClientRPCService.h"

#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {

        void ClientRPCService::GetLeader(protocol::GetLeaderResponse &_return) {
            _return.leader_id = raft_state->GetLeader();
            cout << " recv GET LEADER " << _return.leader_id << endl;
        }

    }
}
