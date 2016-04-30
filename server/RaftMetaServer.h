//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTSERVER_H
#define RAFTFS_RAFTSERVER_H

#include <memory>
#include <server/TThreadPoolServer.h>

#include "RaftConsensus.h"
#include "../filesystem/FSNamespace.h"
#include "../utils/Options.h"

namespace THserver = apache::thrift::server;

namespace raftfs {

    namespace server {

        class RaftMetaServer {
        public:
            RaftMetaServer(Options &opt) ;
            ~RaftMetaServer();


            void Run();

        private:
            void InitRPCServer(int _port, int workder);
        private:
            std::shared_ptr<RaftConsensus> raft_state;
            THserver::TThreadPoolServer* rpc_server;
            std::shared_ptr<filesystem::FSNamespace> fs_namespace;

        };
    }
}


#endif //RAFTFS_RAFTSERVER_H
