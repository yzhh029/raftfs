//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_RAFTRPCSERVICE_H
#define RAFTFS_RAFTRPCSERVICE_H

#include "../protocol/RaftService.h"

namespace raftfs {
    namespace  server {
        class RaftRPCService : virtual public protocol::RaftServiceIf {
        public:
            RaftRPCService() {}

            void RequestVote(protocol::ReqVoteResponse &_return, const protocol::ReqVoteRequest &vote) override ;

            void AppendEntries(protocol::AppendEntriesResponse &_return,
                                       const protocol::AppendEntriesRequest &append) override;


        };
    }
}



#endif //RAFTFS_RAFTRPCSERVICE_H
