//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTSERVER_H
#define RAFTFS_RAFTSERVER_H

#include <memory>
#include "MetaRPCServer.h"


namespace raftfs {

    namespace server {

        class RaftMetaServer {
        public:
            RaftMetaServer() : rpcServer() {}
            void run();

        private:
            MetaRPCServer rpcServer;
        };
    }
}


#endif //RAFTFS_RAFTSERVER_H
