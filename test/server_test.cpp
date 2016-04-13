//
// Created by yzhh on 3/30/16.
//

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TMultiplexedProtocol.h>
#include <protocol/TCompactProtocol.h>
#include "../protocol/RaftService.h"

#include <iostream>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace raftfs::protocol;
using namespace std;

int main() {

    boost::shared_ptr<TSocket> sock(new TSocket("xinu01.cs.purdue.edu", 12345));
    boost::shared_ptr<TCompactProtocol> proto(new TCompactProtocol(sock));
    boost::shared_ptr<TMultiplexedProtocol> raft_proto(
            new TMultiplexedProtocol(proto, "Raft")
    );

    RaftServiceClient client(raft_proto);

    try {
        sock->open();

        ReqVoteRequest req;
        ReqVoteResponse resp;

        req.term = 1;
        req.candidate_id = 2;
        req.__set_last_log_term(0);
        req.__set_last_log_index(123);

        client.RequestVote(resp, req);

        cout << "recv term" << resp.term << " granted " << resp.vote_granted << endl;

        AppendEntriesRequest ae_req;
        AppendEntriesResponse ae_resp;

        ae_req.term = 123;
        ae_req.leader_id = 345;
        ae_req.prev_log_index = 111;
        ae_req.prev_log_term = 132849;
        ae_req.leader_commit_index = 4234;

        client.AppendEntries(ae_resp, ae_req);
        cout << " recv ae term " << ae_resp.term << " success " << ae_resp.success << endl;


        sock->close();
    } catch (TException &tx) {
        cout << tx.what() << endl;
    }


    return 0;
}
