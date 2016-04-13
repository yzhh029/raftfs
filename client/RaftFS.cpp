//
// Created by yzhh on 4/10/16.
// - File System Client
//

#include "RaftFS.h"
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TCompactProtocol.h>
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
        ResetRPCClient(follower_rpc, hosts[rand() % hosts.size()]);

        cout << "ask " << follower_sock->getHost() << endl;
        // get current leader
        GetLeader();
        if (leader_id != -1)
            cout << "current leader is" << hosts[leader_id] << endl;
        else
            cout << "no leader" << endl;

    }


    FSClient::~FSClient() {
        if (follower_sock != nullptr && follower_sock->isOpen())
            follower_sock->close();

        if (leader_sock != nullptr && leader_sock->isOpen())
            leader_sock->close();
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


    void FSClient::CheckLeaders() {
        for (auto& h : hosts) {
            ResetRPCClient(follower_rpc, h);
            cout << " ask " << h << endl;
            leader_id = -1;
            GetLeader();
        }
    }


    void FSClient::ResetSock(boost::shared_ptr<THt::TSocket> &sock, string host) {
        if (sock != nullptr && sock->isOpen())
            sock->close();
        sock.reset(new TSocket(host, port));
    }

    void FSClient::ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client, string host) {
        ResetSock(follower_sock, host);
        //boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(follower_sock));
        boost::shared_ptr<TCompactProtocol> proto(new TCompactProtocol(follower_sock));
        boost::shared_ptr<TMultiplexedProtocol> client_proto(
                new TMultiplexedProtocol(proto, "FSClient")
        );

        follower_rpc.reset(new raftfs::protocol::ClientServiceClient(client_proto));
    }


}
