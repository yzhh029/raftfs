//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTRPCSERVER_H
#define RAFTFS_RAFTRPCSERVER_H

#include "RaftConsensus.h"

// thrift headers
#include <server/TSimpleServer.h>
#include <memory>

namespace TH = apache::thrift;

namespace raftfs {

    namespace server {

        /*
         * MetaRPCServer accepts RPC calls from both other metaserver and client
         * It register:
         *      RaftRPCService
         *      ClientRPCService
         */

        class MetaRPCServer {
        public:
            MetaRPCServer(int _port, RaftConsensus* _raft_state );

            ~MetaRPCServer();
        private:
            const int port;
            std::unique_ptr<TH::server::TSimpleServer> rpc_server;
            RaftConsensus* raft_state;

        };

    }
}



#endif //RAFTFS_RAFTRPCSERVER_H
