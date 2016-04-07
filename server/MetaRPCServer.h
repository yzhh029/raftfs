//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTRPCSERVER_H
#define RAFTFS_RAFTRPCSERVER_H

#include "RaftConsensus.h"

// thrift headers
#include <server/TThreadedServer.h>
#include <server/TThreadPoolServer.h>
#include <server/TSimpleServer.h>
#include <memory>

namespace TH = apache::thrift;

namespace raftfs {

    namespace server {

        /*
         * MetaRPCServer accepts and issue RPC calls from both other metaserver and client
         * It register:
         *      RaftRPCService
         *      ClientRPCService
         *
         */

        class MetaRPCServer {
        public:
            MetaRPCServer(int _port, std::shared_ptr<RaftConsensus> _raft_state );

            void run();
            ~MetaRPCServer() ;
        private:
            std::shared_ptr<RaftConsensus> raft_state;
            TH::server::TThreadPoolServer* rpc_server;
        };

    }
}



#endif //RAFTFS_RAFTRPCSERVER_H
