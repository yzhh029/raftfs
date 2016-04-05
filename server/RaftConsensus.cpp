//
// Created by yzhh on 3/30/16.
//

#include "RaftConsensus.h"
#include "../protocol/RaftService.h"
#include <iostream>
#include <random>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TMultiplexedProtocol.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace raftfs {
    namespace server {

        RemoteHost::RemoteHost(std::string host_name, int port) :
            host(host_name)
        {

            sock.reset(new TSocket(host, port));
            //sock->setRecvTimeout(300);
            boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(sock));
            boost::shared_ptr<TMultiplexedProtocol> raft_proto(
                    new TMultiplexedProtocol(proto, "Raft")
            );
            rpc_client = std::make_shared<raftfs::protocol::RaftServiceClient>(raft_proto);

            //sock->open();
        }


        RaftConsensus::RaftConsensus(Options opt)
            : current_term(0),
              current_role(Role::kFollower),
              stop(false),
              self_id(opt.GetSelfId()),
              self_name(opt.GetSelfName())
        {
            cout << " consensus init " << endl;
            cout << self_id << " " << self_name << endl;

            for (auto &h: opt.GetHosts()) {
                cout << h.first << " " << h.second << endl;
                remotes[h.first] = std::make_shared<RemoteHost>(h.second, opt.GetPort());
            }
        }


        void RaftConsensus::StartRemoteLoops() {
            for (auto &remote : remotes) {
                cout << "starting " << remote.first << " " << remote.second->GetName() << endl;
                thread(&RaftConsensus::RemoteHostLoop, this, remotes[remote.first]).detach();
            }
        }


        void RaftConsensus::CheckLeaderLoop() {
            random_device rd;
            mt19937 gen(rd());

            int upper_bound = 300;
            int lower_bound = 150;

            uniform_int_distribution<> dis(lower_bound, upper_bound);

            while (!stop) {
                unique_lock<mutex> lock(m);
                if (current_role != Role::kLeader &&
                        chrono::steady_clock::now() > next_election) {
                    cout << "leader timeout" << endl;
                } else {
                    next_election += chrono::milliseconds(dis(gen));
                }
                new_event.wait_until(lock, next_election);
            }
        }


        void RaftConsensus::RemoteHostLoop(std::shared_ptr<RemoteHost> remote) {

            string name = remote->GetName();
            cout << "rpc thread to " << name << " started" << endl;

            this_thread::sleep_for(chrono::seconds(2));

            auto rpc_client = remote->GetRPCClient();

            while (!stop) {
                //cout << "rpc "<< name << "go sleep" << endl;
                //this_thread::yield();
                this_thread::sleep_for(chrono::seconds(1));
                //unique_lock<mutex> lock(m);
                cout << "rpc "<< name << "wake up" << endl;
                switch (current_role) {
                    case Role::kLeader:
                        break;
                    case Role::kFollower:
                        break;
                    case Role::kCandidate:
                        break;
                }
                protocol::AppendEntriesRequest req;
                protocol::AppendEntriesResponse resp;

                req.term = 123;
                req.leader_id = 345;
                req.prev_log_index = 111;
                req.prev_log_term = 132849;
                req.leader_commit_index = 4234;

                //ock.unlock();
                auto start = chrono::steady_clock::now();
                try {
                    if (remote->Connected())
                        rpc_client->AppendEntries(resp, req);
                    else {
                        remote->TryConnect();
                        if (remote->Connected())
                            rpc_client->AppendEntries(resp, req);
                    }
                } catch (transport::TTransportException te) {
                    //cout << te.what() << endl;
                    cout << "lost communication to " << name << endl;
                    continue;
                }


                auto end = chrono::steady_clock::now();

                cout << "append entries to " << name << " recv ae term" << resp.term
                    << " success " << resp.success << " in " << chrono::duration_cast<chrono::milliseconds>(end-start).count() << " ms" << endl;

                //lock.lock();
                //new_event.wait_for(lock, chrono::milliseconds(300));
            }
        }

    }
}