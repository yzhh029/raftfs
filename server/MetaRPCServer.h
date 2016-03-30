//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTRPCSERVER_H
#define RAFTFS_RAFTRPCSERVER_H

// thrift headers
#include <server/TSimpleServer.h>


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
            MetaRPCServer(int _port);

            ~MetaRPCServer();
        private:
            const int port;
            TH::server::TSimpleServer rpc_server;

        };

    }
}



#endif //RAFTFS_RAFTRPCSERVER_H
