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
              next_election(Now())
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

            //this_thread::sleep_for(chrono::seconds(1)); // currently 1sec.
            PostponeElection();

            while (!stop) {
                unique_lock<mutex> lock(m);
                if (current_role != Role::kLeader &&
                        Now() >= next_election) {
                    cout << TimePointStr(Now())<< " leader timeout " << endl;
                    StartLeaderElection();
                    PostponeElection();
                    new_event.notify_all();
                }
                new_event.wait_until(lock, next_election);
            }
        }


        void RaftConsensus::RemoteHostLoop(std::shared_ptr<RemoteHost> remote) {

            string name = remote->GetName();
            auto id = remote->GetID();
            cout << "rpc thread to " << name << " started" << endl;

            //this_thread::sleep_for(chrono::seconds(1));

            auto rpc_client = remote->GetRPCClient();

            while (!stop) {

                unique_lock<mutex> lock(m);
                new_event.wait_for(lock, chrono::seconds(1));
                //cout << "rpc "<< name << "wake up" << endl;
                switch (current_role) {
                    case Role::kLeader: {
                        protocol::AppendEntriesRequest ae_req;
                        protocol::AppendEntriesResponse ae_resp;

                        ae_req.term = current_term;
                        ae_req.leader_id = leader_id;
                        ae_req.prev_log_index = GetLastLogIndex();
                        ae_req.prev_log_term = GetLastLogTerm();
                        ae_req.leader_commit_index = GetLastLogIndex();
                        cout << TimePointStr(Now()) << " ae req to " << id << " T:" << current_term
                            << " L:" << leader_id << endl;
                        lock.unlock();
                        try {
                            if (remote->Connected())
                                rpc_client->AppendEntries(ae_resp, ae_req);
                        } catch (transport::TTransportException te) {
                            //cout << te.what() << endl;
                            cout << "lost communication to " << name << endl;
                            continue;
                        }
                        lock.lock();
                        cout << TimePointStr(Now()) << " ae resp from " << id << " RT:" << ae_resp.term
                        << " S: " << ae_resp.success << endl;


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

                        cout << TimePointStr(Now()) << " ask vote to " << id << " T:" << current_term << endl;
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
                        cout << TimePointStr(Now()) << " granted? " << resp.vote_granted << " from " << id
                             << " LT" << current_term << " RT" << resp.term << endl;
                        if (current_role == Role::kCandidate && resp.vote_granted && resp.term <= current_term) {
                            vote_pool.insert(remote->GetID());
                            cout << TimePointStr(Now()) << " updated pool size:" << vote_pool.size() << endl;
                            // have enough votes
                            if (current_role == Role::kCandidate && vote_pool.size() >= quorum_size) {
                                //  change to leader
                                cout << TimePointStr(Now()) << " win leader election LT:" << current_term << endl;
                                ChangeToLeader();
                            }
                        }
                        break;
                    }
                }
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

            cout << TimePointStr(Now()) << " new election for T:" << current_term << endl;
        }

        // Set next leader election timer.
        void RaftConsensus::PostponeElection() {
            static random_device rd;
            static mt19937 gen(rd());

            static int upper_bound = 3000;
            static int lower_bound = 1500;

            static uniform_int_distribution<> dis(lower_bound, upper_bound);

            next_election = Now() + chrono::milliseconds(dis(gen));
            //next_election = Now() + chrono::seconds(2);
        }

        // Change our role to leader and update leader ID.
        void RaftConsensus::ChangeToLeader() {
            current_role = Role::kLeader;
            leader_id = self_id;
        }


        void RaftConsensus::ChangeToFollower(int64_t new_term) {
            if (current_role == Role::kCandidate) {
                vote_pool.clear();
            }
            current_role = Role::kFollower;
            current_term = new_term;
            leader_id = -1;
        }


        // Handle RPC: AppendEntries()
        void RaftConsensus::OnAppendEntries(protocol::AppendEntriesResponse &resp,
                                            const protocol::AppendEntriesRequest &req) {
            lock_guard<mutex> lock(m);
            bool success = true;

            // if sender has higher term, receiver catch up his term and change to follower
            if (req.term > current_term) {
                resp.term = current_term;
                ChangeToFollower(req.term);
                leader_id = req.leader_id;
                cout << TimePointStr(Now()) <<" OnAE new leader " << leader_id << " RT:" << req.term << " change to follower" << endl;

            } else if (current_role == Role::kCandidate && req.term == current_term) {
                // have a new leader
                ChangeToFollower(req.term);
                leader_id = req.leader_id;
                cout << TimePointStr(Now()) <<" OnAE new leader " << leader_id << " RT:" << req.term << " change to follower" << endl;
                //PostponeElection();
                resp.term = current_term;
            } else if (current_term > req.term) {
                // reject rpc
                resp.term = current_term;
                success = false;
            }
            PostponeElection();

            //resp.term = current_term;
            resp.success = success;

            // TODO log operations

        }

        // Handle RPC: OnRequestVote
        void RaftConsensus::OnRequestVote(protocol::ReqVoteResponse &resp, const protocol::ReqVoteRequest &req) {
            lock_guard<mutex> lock(m);

            int64_t lastlogterm = GetLastLogTerm();
            int64_t lastlogindex = GetLastLogIndex();
            bool grant = false;

            if (req.term > current_term) {
                ChangeToFollower(req.term);
                cout << TimePointStr(Now()) <<"OnRV new RT:" << req.term << " change to follower" << endl;
            }

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
            if (!grant) {
                cout << "reject vote to" << req.candidate_id << " term:" << req.term << endl;
            }

            resp.term = current_term;
            resp.vote_granted = grant;
        }


    }
}
