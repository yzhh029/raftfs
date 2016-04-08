//
// Created by yzhh on 3/30/16.
//

#include "RaftConsensus.h"
#include "../utils/time_utils.h"
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

        RemoteHost::RemoteHost(int32_t _id, std::string host_name, int port) :
            host(host_name), id(_id)
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
              self_name(opt.GetSelfName()),
              leader_id(-1),
              next_election(Now() + chrono::seconds(1))
        {
            cout << " consensus init " << endl;
            cout << self_id << " " << self_name << endl;

            for (auto &h: opt.GetHosts()) {
                cout << h.first << " " << h.second << endl;
                remotes[h.first] = std::make_shared<RemoteHost>(h.first, h.second, opt.GetPort());
            }

            quorum_size = (remotes.size() + 1) / 2 + 1;

            // add an emtry log entry
            protocol::Entry entry;
            entry.index = 0;
            entry.term = current_term;
            log.push_back(entry);
        }


        void RaftConsensus::StartRemoteLoops() {
            for (auto &remote : remotes) {
                cout << "starting " << remote.first << " " << remote.second->GetName() << endl;
                thread(&RaftConsensus::RemoteHostLoop, this, remotes[remote.first]).detach();
            }
        }


        void RaftConsensus::StartLeaderCheckLoop() {
            thread(&RaftConsensus::CheckLeaderLoop, this).detach();
        }


        void RaftConsensus::CheckLeaderLoop() {

            this_thread::sleep_for(chrono::seconds(1));

            while (!stop) {
                unique_lock<mutex> lock(m);
                if (current_role != Role::kLeader &&
                        Now() >= next_election) {
                    cout << "leader timeout" << endl;
                    StartLeaderElection();
                    new_event.notify_all();
                } else {
                    PostponeElection();
                }
                new_event.wait_until(lock, next_election);
            }
        }


        void RaftConsensus::RemoteHostLoop(std::shared_ptr<RemoteHost> remote) {

            string name = remote->GetName();
            cout << "rpc thread to " << name << " started" << endl;

            this_thread::sleep_for(chrono::seconds(1));

            auto rpc_client = remote->GetRPCClient();

            while (!stop) {

                unique_lock<mutex> lock(m);
                new_event.wait_for(lock, chrono::seconds(1));
                cout << "rpc "<< name << "wake up" << endl;
                switch (current_role) {
                    case Role::kLeader: {
                        protocol::AppendEntriesRequest ae_req;
                        protocol::AppendEntriesResponse ae_resp;

                        ae_req.term = 123;
                        ae_req.leader_id = 345;
                        ae_req.prev_log_index = 111;
                        ae_req.prev_log_term = 132849;
                        ae_req.leader_commit_index = 4234;
                        try {
                            if (remote->Connected())
                                rpc_client->AppendEntries(ae_resp, ae_req);
                        } catch (transport::TTransportException te) {
                            //cout << te.what() << endl;
                            cout << "lost communication to " << name << endl;
                            continue;
                        }
                        cout << "append entries to " << name << " recv ae term" << ae_resp.term
                        << " success " << ae_resp.success << endl;

                        break;
                    }
                    case Role::kFollower: {


                        break;
                    }
                    case Role::kCandidate: {
                        // construct message
                        protocol::ReqVoteRequest req;
                        protocol::ReqVoteResponse resp;
                        req.candidate_id = self_id;
                        req.term = current_term;
                        req.last_log_index = GetLastLogIndex();
                        req.last_log_term = GetLastLogTerm();

                        // do rpc call
                        lock.unlock();
                        try {
                            if (remote->Connected())
                                rpc_client->RequestVote(resp, req);
                        } catch (transport::TTransportException te) {
                            continue;
                        }
                        // update vote result
                        lock.lock();
                        if (resp.vote_granted && resp.term == current_term) {
                            vote_pool.insert(remote->GetID());
                            // have enough votes
                            if (current_role == Role::kCandidate && vote_pool.size() >= quorum_size) {
                                //  change to leader
                                ChangeToLeader();
                            }
                        }
                        break;
                    }
                }

                /*
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
                } catch (transport::TTransportException te) {
                    //cout << te.what() << endl;
                    cout << "lost communication to " << name << endl;
                    continue;
                }


                auto end = chrono::steady_clock::now();

                cout << "append entries to " << name << " recv ae term" << resp.term
                    << " success " << resp.success << " in " << chrono::duration_cast<chrono::milliseconds>(end-start).count() << " ms" << endl;
                */
                //lock.lock();
                //new_event.wait_for(lock, chrono::milliseconds(300));
            }
        }

        void RaftConsensus::StartLeaderElection() {
            ++current_term;
            current_role = Role::kCandidate;
            leader_id = -1;
            vote_pool.clear();
            if (vote_for.find(current_term) == vote_for.end()) {
                vote_for[current_term] = self_id;
                vote_pool.insert(self_id);
            }

            cout << "new election T:" << current_term << endl;
        }


        void RaftConsensus::PostponeElection() {
            static random_device rd;
            static mt19937 gen(rd());

            static int upper_bound = 300;
            static int lower_bound = 150;

            static uniform_int_distribution<> dis(lower_bound, upper_bound);

            //next_election = Now() + chrono::milliseconds(dis(gen));
            next_election = Now() + chrono::seconds(2);
        }


        void RaftConsensus::ChangeToLeader() {
            current_role = Role::kLeader;
            leader_id = self_id;
        }


        void RaftConsensus::OnAppendEntries(protocol::AppendEntriesResponse &resp,
                                            const protocol::AppendEntriesRequest &req) {
            lock_guard<mutex> lock(m);
            bool success = true;

            if (current_role == Role::kCandidate && req.term >= current_term) {
                // have a new leader
                leader_id = req.leader_id;
                current_role = Role::kFollower;
                PostponeElection();
            } else if (current_term > req.term) {
                // reject rpc
                success = false;
            }

            resp.term = current_term;
            resp.success = success;

            // TODO log operations

        }

        void RaftConsensus::OnRequestVote(protocol::ReqVoteResponse &resp, const protocol::ReqVoteRequest &req) {
            lock_guard<mutex> lock(m);

            int64_t lastlogterm = GetLastLogTerm();
            int64_t lastlogindex = GetLastLogIndex();
            bool grant = false;
            // make sure candidate's log is up to date
            if (lastlogindex <= req.last_log_index
                    && lastlogterm <= req.last_log_term) {
                auto vote = vote_for.find(req.term);

                if (vote == vote_for.end() || (vote != vote_for.end() && vote->second == req.candidate_id)) {
                    cout << "grant vote to " << req.candidate_id << " term:" << req.term << endl;
                    grant = true;
                    vote_for[req.term] = req.candidate_id;
                }
            }

            resp.term = current_term;
            resp.vote_granted = grant;
        }


    }
}