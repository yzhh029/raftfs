//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTRPCSERVICE_H
#define RAFTFS_RAFTRPCSERVICE_H

#include "RaftConsensus.h"
#include "../protocol/RaftService.h"
#include <memory>

namespace raftfs {
    namespace  server {
        class RaftRPCService : virtual public protocol::RaftServiceIf {
        public:
            RaftRPCService(RaftConsensus& state) :
                    raft_state(state){}

            void RequestVote(protocol::ReqVoteResponse &_return, const protocol::ReqVoteRequest &vote) override ;

            void AppendEntries(protocol::AppendEntriesResponse &_return,
                                       const protocol::AppendEntriesRequest &append) override;

        private:
            RaftConsensus& raft_state;
        };
    }
}



#endif //RAFTFS_RAFTRPCSERVICE_H
