//
// Created by yzhh on 4/10/16.
//

#include "RaftFS.h"
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TMultiplexedProtocol.h>

using namespace std;
using namespace apache::thrift::transport;
using namespace apache::thrift;
using namespace apache::thrift::protocol;

namespace raftfs {

    FSClient::FSClient(Options &opt)
            : port(opt.GetPort()),
              hosts(opt.GetAllHosts()),
              leader_id(-1)
    {
        ResetSock(follower_sock, hosts[rand() % hosts.size()]);
        boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(follower_sock));
        boost::shared_ptr<TMultiplexedProtocol> client_proto(
                new TMultiplexedProtocol(proto, "FSClient")
        );

        follower_rpc = std::make_shared<raftfs::protocol::ClientServiceClient>(client_proto);

        cout << "ask " << follower_sock->getHost() << endl;
        // get current leader
        GetLeader();
        if (leader_id != -1)
            cout << "current leader is" << hosts[leader_id] << endl;
        else
            cout << "no leader" << endl;

    }

    int32_t FSClient::GetLeader() {
        if (leader_id == -1) {
            if (!follower_sock->isOpen())
                follower_sock->open();
            protocol::GetLeaderResponse resp;

            follower_rpc->GetLeader(resp);
            leader_id = resp.leader_id;
            cout << " new leader is " << leader_id << endl;
        }
        return leader_id;
    }


    void FSClient::ResetSock(boost::shared_ptr<THt::TSocket> &sock, string host) {
        sock.reset(new TSocket(host, port));
    }

    void FSClient::ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client, string host) {

    }


}
