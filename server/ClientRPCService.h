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
            ClientRPCService(std::shared_ptr<RaftConsensus>& state):
                    raft_state(state) {}

            void GetLeader(protocol::GetLeaderResponse &_return) override ;

        private:
            std::shared_ptr<RaftConsensus>& raft_state;
        };
    }
}

#endif //RAFTFS_CLIENTRPCSERVICE_H
