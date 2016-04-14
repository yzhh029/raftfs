//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_CLIENTRPCSERVICE_H
#define RAFTFS_CLIENTRPCSERVICE_H

#include "RaftConsensus.h"
#include "../protocol/ClientService.h"
#include <memory>

namespace raftfs {
    namespace server {
        class ClientRPCService : virtual public protocol::ClientServiceIf {
        public:
            ClientRPCService(RaftConsensus& state):
                    raft_state(state) {}

            void GetLeader(protocol::GetLeaderResponse &_return) override ;

            void Mkdir(protocol::MkdirResponse &_return, const protocol::MkdirRequest &new_dir) override;

        private:
            RaftConsensus& raft_state;
        };
    }
}

#endif //RAFTFS_CLIENTRPCSERVICE_H
