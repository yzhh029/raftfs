//
// Created by yzhh on 3/30/16.
//

#include "RaftConsensus.h"
#include "../utils/time_utils.h"
#include "../protocol/RaftService.h"
#include <iostream>
#include <random>
#include <cassert>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TMultiplexedProtocol.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace raftfs::filesystem;

namespace raftfs {
    namespace server {

        RemoteHost::RemoteHost(int32_t _id, std::string host_name, int port) :
            host(host_name), id(_id), match_index(-1)
        {

            sock.reset(new TSocket(host, port));
            //sock->setRecvTimeout(300);
            //boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(sock));
            boost::shared_ptr<TCompactProtocol> proto(new TCompactProtocol(sock));
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

            PostponeElection();
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


        void RaftConsensus::StartFSUpdateLoop(FSNamespace *fs) {
            thread(&RaftConsensus::FSUpdateLoop, this, fs).detach();
        }

        /*
         * Thread 1 in raft_server
         */
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

                if (current_role != Role::kLeader)
                    new_event.wait_until(lock, next_election);
                else
                    new_event.wait_until(lock, chrono::system_clock::time_point::max());
            }
        }

        /*
         * Thread 2 in raft_server
         */
        void RaftConsensus::RemoteHostLoop(std::shared_ptr<RemoteHost> remote) {

            string name = remote->GetName();
            auto id = remote->GetID();
            cout << "rpc thread to " << name << " started" << endl;

            auto rpc_client = remote->GetRPCClient();
            unique_lock<mutex> lock(m);

            while (!stop) {
                //cout << "rpc "<< name << "wake up" << endl;
                switch (current_role) {
                	/*
                	 * When perform as leaeder, it will do two things:
                	 * 1: sent empty append entries request to remote host as heartbeat
                	 * 2: sent non-sync entries to remote to replicate log
                	 */
                    case Role::kLeader: {
                        protocol::AppendEntriesRequest ae_req;
                        protocol::AppendEntriesResponse ae_resp;

                        ae_req.term = current_term;
                        ae_req.leader_id = self_id;
                        ae_req.prev_log_term = log.GetLastLogTerm();
                        ae_req.leader_commit_index = log.GetLastCommitIndex();
                        //-- Current log index at remote host.
                        ae_req.prev_log_index = remote->GetNextIndex() - 1;

                        cout << TimePointStr(Now()) << " ae req to " << id << " T:" << current_term
                            << " L:" << leader_id << " prev:" << ae_req.prev_log_index << " C:" << ae_req.leader_commit_index<< endl;
                        // found one or more new log entry, add them to the request
                        // TODO: limit maximum entry size to limit request size.
                        if (ae_req.prev_log_index < log.GetLastLogIndex()) {
                            cout <<"r"<< id << " new entries prev:" << ae_req.prev_log_index  << endl;

                            // Copy new entries into request.
                            ae_req.__set_entries(log.GetEntriesStartAt(ae_req.prev_log_index));
                            // Print out new entries for information.
                            for (auto &e : ae_req.entries) {
                                cout << id << "   I:" << e.index << " " << e.op << " " << e.value << endl;
                            }
                        }
                        lock.unlock();
                        //-- Push entries to remote --
                        try {
                            if (remote->Connected()) {
                            	/* Append entries to remote servers.
                            	 * - Calls RaftServiceClient::AppendEntries()
                            	 *   IN: ae_req;	OUT: ae_resp
                            	 */
                                rpc_client->AppendEntries(ae_resp, ae_req);

                            }

                        } catch (transport::TTransportException te) {
                            cout << "lost communication to " << name << endl;
                            lock.lock();
                            break;
                        }
                        lock.lock();

                        cout << TimePointStr(Now()) << " ae resp from " << id << " RT:" << ae_resp.term
                        << " S: " << ae_resp.success << endl;
                        // deal with response.
                        // have normal ae response
                        if (ae_resp.term == current_term) {

                            if (ae_resp.success) {
                                if (!ae_req.entries.empty()) {
                                    // append new entries success
                                    cout << "r" << id << " update next index " << ae_resp.last_log_index + 1 << endl;
                                    // update remote next index
                                    remote->ResetNextIndex(ae_resp.last_log_index + 1);

                                    auto commit_index = log.GetLastCommitIndex();
                                    int64_t new_commit = commit_index;
                                    for (auto& e : ae_req.entries ) {
                                        // skip commited entry
                                        if (commit_index >= e.index)
                                            continue;
                                        // update quorum status
                                        pending_entries[e.index].insert(id);
                                        if (pending_entries[e.index].size() >= quorum_size) {
                                            new_commit = e.index;
                                        }
                                    }
                                    // debug
                                    //cout << "log: " << log << endl;

                                    // update commit index
                                    if (commit_index != new_commit) {
                                        log.SetLastCommitIndex(new_commit);
                                        cout << TimePointStr(Now()) << " new log commit index " << new_commit << endl;
                                        cout << "log: " << endl << log << endl;
                                        fs_commit.notify_all();
                                    }
                                } else {
                                    // normal heartbeat response also goes here
                                    // for now do nothing
                                }
                            } else {
                                // the reason of failed rpc is because remote cannot find an entry that has
                                // the same prev_index and prev_term
                                // backoff the next_index
                                // to reduce the rpc round to find sync entry, backoff 5 entries every time
                                ae_resp.printTo(cout);
                                if (ae_req.prev_log_index > 5) {
                                    remote->ResetNextIndex(ae_req.prev_log_index - 5);
                                } else {
                                    remote->ResetNextIndex(1);
                                }
                                cout << "reset next index to" << remote->GetNextIndex() << endl;
                                // skip wait, do next round ae rpc immediately
                                continue;
                            }
                        } else if (ae_resp.term > current_term) {
                            // if follower has higher term, change to follower
                            // TODO: discard uncommited entires
                            ChangeToFollower(ae_resp.term);
                        }

                        break;
                    }
                    //-----------------------------------------------------
                    /*
                     * follwer never send rpc to anyone
                     */
                    case Role::kFollower: {


                        break;
                    }
                    //-----------------------------------------------------
                    case Role::kCandidate: {
                        // construct message
                        protocol::ReqVoteRequest req;
                        protocol::ReqVoteResponse resp;
                        req.candidate_id = self_id;
                        req.term = current_term;
                        req.last_log_index = log.GetLastLogIndex();
                        req.last_log_term = log.GetLastLogTerm();

                        cout << TimePointStr(Now()) << " ask vote to " << id << " T:" << current_term << endl;
                        // do rpc call
                        lock.unlock();
                        try {
                            if (remote->Connected())
                            	rpc_client->RequestVote(resp, req);
                        } catch (transport::TTransportException te) {
                            lock.lock();
                            break;
                        }
                        // update vote result
                        lock.lock();
                        if (current_role == Role::kCandidate)
                            cout << TimePointStr(Now()) << " granted? " << resp.vote_granted << " from " << id
                                << " LT" << current_term << " RT" << resp.term << endl;
                        else
                            continue;
                        if (current_role == Role::kCandidate && resp.vote_granted && resp.term <= current_term) {
                            vote_pool.insert(remote->GetID());
                            cout << TimePointStr(Now()) << " updated pool size:" << vote_pool.size() << endl;
                            // have enough votes
                            if ( vote_pool.size() >= quorum_size) {
                                //  change to leader
                                cout << TimePointStr(Now()) << " win leader election LT:" << current_term << endl;
                                ChangeToLeader();
                                continue;
                            }
                        }
                        break;
                    }
                }
                // Wait for new event and process next loop.
                new_event.wait_for(lock, chrono::milliseconds(250));

            }
        }


        void RaftConsensus::FSUpdateLoop(FSNamespace *fs) {

            using namespace raftfs::protocol;

            unique_lock<mutex> lock(fs_m);

            while (!stop) {
                fs_commit.wait(lock, [this, fs]() {
                    return log.GetLastCommitIndex() != fs->GetCommitedIndex();
                });
                int64_t commited = 0;
                for (int64_t i = fs->GetCommitedIndex() + 1; i <= log.GetLastCommitIndex(); ++i) {
                    if ((IsLeader() && pending_entries[i].size() >= quorum_size) || IsFollower()) {
                        const Entry* e = log.GetEntry(i);
                        cout << TimePointStr(Now()) << " FS try apply " << e->index << " " << OpToStr(e->op) << " " << e->value << endl;
                        switch (e->op) {
                            case MetaOp::kMkdir: {
                                // todo maybe add another entry field called client/operator
                                fs->MakeDir(e->value, string("unknown"), true);
                                cout << TimePointStr(Now()) << " commit I:" << e->index << " mkdir " << e->value << endl;
                                break;
                            }
                            case MetaOp::kRmdir: {
                                fs->DeleteDir(e->value, string("unknown"), true);
                                cout << TimePointStr(Now()) << " commit I:" << e->index << " rmdir " << e->value << endl;
                                break;
                            }
                            case MetaOp::kCreate: {
                                fs->CreateFile(e->value, string("unknown"));
                                cout << TimePointStr(Now()) << " commit I:" << e->index << " create file " << e->value << endl;
                                break;
                            }
                            case MetaOp::kDelete: {
                                fs->RemoveFile(e->value, string("unknown"));
                                cout << TimePointStr(Now()) << " commit I:" << e->index << " delete file " << e->value << endl;
                                break;
                            }
                            default:
                                cout << "UNKNOWN fs op !!!" << e->op << endl;
                        }
                        commited = i;
                        fs->Print();
                    } else {
                        break;
                    }
                }
                cout << TimePointStr(Now()) << " FS committed " << commited - fs->GetCommitedIndex() << " entries" << endl;
                fs->SetCommitedIndex(commited);
                //cout << "fs update wake up commit " << log.GetLastCommitIndex() << " last " << log.GetLastLogIndex() << endl;
                if (IsLeader())
                    client_ready.notify_all();
                //this_thread::sleep_for(chrono::seconds(1));
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

            static int upper_bound = 1000;
            static int lower_bound = 500;

            static uniform_int_distribution<> dis(lower_bound, upper_bound);

            next_election = Now() + chrono::milliseconds(dis(gen));

        }

        // Change our role to leader and update leader ID.
        void RaftConsensus::ChangeToLeader() {
            current_role = Role::kLeader;
            leader_id = self_id;

            int64_t next_index = log.GetLastLogIndex() + 1;
            for (auto &r : remotes) {
                r.second->ResetNextIndex(next_index);
                //cout << r.second->GetNextIndex();
            }

            // notify all peer thread to send AE
            new_event.notify_all();
        }


        void RaftConsensus::ChangeToFollower(int64_t new_term) {
            if (current_role == Role::kCandidate) {
                vote_pool.clear();
            }
            current_role = Role::kFollower;
            current_term = new_term;
            leader_id = -1;

            PostponeElection();
            new_event.notify_all();
        }

        // Handle RPC: AppendEntries()
        void RaftConsensus::OnAppendEntries(protocol::AppendEntriesResponse &resp,
                                            const protocol::AppendEntriesRequest &req) {
            lock_guard<mutex> lock(m);

            bool success = true;
            resp.term = current_term;

            // deal with term
            // if receive an outdated term from caller, reject it and end function
            if (current_term > req.term) {
                success = false;
                cout << TimePointStr((Now())) << " OnAE slow term reject" << endl;
                return;
            } else {
                // receive a valid AE request from leader, postpone election timer first
                PostponeElection();

                // if sender has higher term, receiver catch up his term and change to follower
                // Or failed to be the first leader in election term, change to follower
                if (req.term > current_term
                        || (current_role == Role::kCandidate && req.term >= current_term)) {
                    cout << TimePointStr(Now()) <<" OnAE new leader" << req.leader_id << " RT:" << req.term << " change to follower" << endl;
                    if (req.term > current_term) {
                        resp.term = current_term;
                    }
                    ChangeToFollower(req.term);
                    leader_id = req.leader_id;
                }
            }

            // after the term check, if we have a valid request, then preceed to operate on log entires
            // leader.term >= local term
            if (success) {
                // update leader id if not set
                if (leader_id == -1) {
                    leader_id = req.leader_id;
                }

                // log operations
                // log index check:
                //      if local last > leader prev, means have conflict entries
                //      if local last == leader prev, no conflict
                if (log.GetLastLogIndex() >= req.prev_log_index ) {
                    // try to append all new entires to local log
                    if (!req.entries.empty()) {
                        cout << TimePointStr(Now()) << " new entries from leader size:" << req.entries.size() << endl;
                        for (auto& e : req.entries) {
                            cout << "   I:" << e.index << " " << e.op << " " << e.value << endl;
                        }
                        success = log.Append(&req.entries);

                        //debug
                        if (success) {
                            cout << TimePointStr(Now()) << " append " << req.entries.size()
                            << "entires SUCC LI:" << log.GetLastLogIndex() << endl;
                        }
                        else
                            cout << TimePointStr(Now()) << " append " << req.entries.size()
                                    << "entires FAIL LI:" << log.GetLastLogIndex() << endl;
                    }
                } else {
                    // if prev > last_index means there will be a gap in log
                    // should not allow such operation
                	cout << TimePointStr(Now()) << " forbid gap Local:" << log.GetLastLogIndex()
                        << " Remote:" << req.prev_log_index << endl;

                    // reject this request and report local last index
                    success = false;
                    resp.__set_last_log_index(log.GetLastLogIndex());
                }
				// TODO: update commit index and apply commited entries

                if (req.leader_commit_index > log.GetLastCommitIndex()) {
                    log.SetLastCommitIndex(req.leader_commit_index);
                    cout << "new commit leaderC:" << req.leader_commit_index << " localC:" << log.GetLastCommitIndex() << endl;
                    fs_commit.notify_all();
                }

            }

            // debug output
            if (!req.entries.empty())
                cout << "log: " << log << endl;

            resp.success = success;
            if (success)
                resp.__set_last_log_index(log.GetLastLogIndex());
        }

        // Handle RPC: OnRequestVote
        void RaftConsensus::OnRequestVote(protocol::ReqVoteResponse &resp, const protocol::ReqVoteRequest &req) {
            lock_guard<mutex> lock(m);

            auto term_index = log.GetLastLogTermAndIndex();
            bool grant = false;

            if (req.term > current_term) {
                ChangeToFollower(req.term);
                cout << TimePointStr(Now()) <<" OnRV new RT:" << req.term << " change to follower" << endl;
            }

            // make sure candidate's log is up to date
            if (term_index.second <= req.last_log_index
                    && term_index.first <= req.last_log_term) {
                auto vote = vote_for.find(req.term);

                if (vote == vote_for.end() || (vote != vote_for.end() && vote->second == req.candidate_id)) {
                    cout << TimePointStr(Now()) << " OnRV grant vote to " << req.candidate_id << " term:" << req.term << endl;
                    grant = true;
                    vote_for[req.term] = req.candidate_id;
                    if (req.term == current_term)
                        PostponeElection();
                }
            }
            if (!grant) {
                cout << TimePointStr(Now()) << " OnRV reject vote to" << req.candidate_id << " term:" << req.term << endl;
            }

            resp.term = current_term;
            resp.vote_granted = grant;
        }


        protocol::Status::type RaftConsensus::OnMetaOperation(protocol::MetaOp::type op, std::string path, void *params) {
            cout << TimePointStr(Now()) << op << " " << path << endl;
            // TODO add op speration
            // follower should only process non modification operation
            //  --> so follwer can response to read-only op.
            if (leader_id == -1) {
                cout << "not leader" << endl;
                return protocol::Status::kNoLeader;
            } else {

                protocol::Entry* e = new protocol::Entry();

                e->op = op;
                e->value = path;
                e->term = current_term;

                log.Append(e);	// Leader's log
                cout << TimePointStr(Now()) << " append new e T:" << e->term
                		<< " I:" << e->index << " LI:" << log.GetLastLogIndex()<< endl;
                new_event.notify_all();	// notify remote servers

                unique_lock<mutex> lock(cli_m);
                if (client_ready.wait_for(lock, chrono::seconds(2), [&] {return e->index <= log.GetLastCommitIndex();})) {
                    // success
                    cout << TimePointStr(Now()) << e->index << " commited!" << endl;
                    return protocol::Status::kOK;
                } else {
                    cout << TimePointStr(Now()) << e->index << "timeout!" << endl;
                    return protocol::Status::kTimeout;
                }
            }
        }


        void RaftConsensus::ChangeLeaderID(int32_t newid) {
            cout << "leader id from " << leader_id << " to " << newid << endl;
            leader_id = newid;
        }

        protocol::Status::type RaftConsensus::OnInjectTestCase(int32_t type) {
        	// TODO: handle test cases.
        	return protocol::Status::kOK;
        }


    }
}
