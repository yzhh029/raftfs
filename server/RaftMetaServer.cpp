//
// Created by yzhh on 3/29/16.
//

#include "RaftMetaServer.h"

#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {

        RaftMetaServer::RaftMetaServer() :
            raft_state(new RaftConsensus()), rpc_server(12345, raft_state) {
            cout << "MetaServer init" << endl;
        }


        void RaftMetaServer::run() {
            cout << "FS server start" << endl;
        }


    }
}


