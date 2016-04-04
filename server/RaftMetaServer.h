//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTSERVER_H
#define RAFTFS_RAFTSERVER_H

#include <memory>
#include "MetaRPCServer.h"
#include "RaftConsensus.h"
#include "../utils/Options.h"


namespace raftfs {

    namespace server {

        class RaftMetaServer {
        public:
            RaftMetaServer(Options &opt) ;
            void Run();

        private:
            std::shared_ptr<RaftConsensus> raft_state;
            MetaRPCServer rpc_server;

        };
    }
}


#endif //RAFTFS_RAFTSERVER_H
