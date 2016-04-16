//
// Created by yzhh on 3/29/16.
//

#include "RaftRPCService.h"

#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {


        void RaftRPCService::RequestVote(protocol::ReqVoteResponse &_return, const protocol::ReqVoteRequest &vote) {
            //cout << "recv vote request" << endl;

            //cout << "term " << vote.term << " candidate id" << vote.candidate_id << endl;
            //cout << "last term " << vote.last_log_term << "last index " << vote.last_log_index << endl;

            raft_state.OnRequestVote(_return, vote);
            //_return.term = vote.term + 1;
            //_return.vote_granted = true;
        }

        // When receive AppendEntries() RPC call from Leader.
        void RaftRPCService::AppendEntries(protocol::AppendEntriesResponse &_return,
                                           const protocol::AppendEntriesRequest &append) {
            //cout << " recv ae request" << endl;
            //cout << " term " << append.term << " leader_id" << append.leader_id << endl;
            raft_state.OnAppendEntries(_return, append);
            //_return.term = append.term + 1;
            //_return.success = true;
        }


    }
}
