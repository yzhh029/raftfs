//
// Created by yzhh on 3/29/16.
//

#include "RaftMetaServer.h"

#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {

        RaftMetaServer::RaftMetaServer(Options &opt) :
                raft_state(new RaftConsensus(opt)),
                rpc_server(opt.GetPort(), raft_state)
        {
            cout << "MetaServer init" << endl;
        }


        void RaftMetaServer::Run() {
            cout << "FS server start" << endl;
            rpc_server.run();
            //std::thread(&MetaRPCServer::run, this);
        }


    }
}


