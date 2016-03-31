//
// Created by yzhh on 3/29/16.
//

#include "RaftRPCService.h"

#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {


        void RaftRPCService::RequestVote(protocol::ReqVoteResponse &_return, const protocol::ReqVoteRequest &vote) {
            cout << "recv vote request" << endl;

            cout << "term " << vote.term << " candidate id" << vote.candidate_id << endl;
            cout << "last term " << vote.last_log_term << "last index " << vote.last_log_index << endl;

            _return = protocol::ReqVoteResponse();
            _return.term = vote.term + 1;
            _return.vote_granted = true;
        }


    }
}