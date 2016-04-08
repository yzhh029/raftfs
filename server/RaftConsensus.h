//
// Created by yzhh on 3/30/16.
//

#ifndef RAFTFS_RAFTCONSENSUS_H
#define RAFTFS_RAFTCONSENSUS_H

#include <stdint.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <map>
#include <unordered_map>
#include <transport/TSocket.h>

#include "../protocol/RaftService.h"
#include "../utils/Options.h"
#include "../utils/time_utils.h"


namespace raftfs {
    namespace server {

        class RemoteHost {  // Remote Host sock & rpc_client

        public:
            RemoteHost(int32_t id, std::string host_name, int port);
            ~RemoteHost() {
                if (sock->isOpen())
                    sock->close();
            }
            std::string GetName() const {return host;}
            int32_t GetID() const {return id;}
            std::shared_ptr<raftfs::protocol::RaftServiceClient> GetRPCClient() {
                return rpc_client;
            }

            void TryConnect() {
                sock->open();
            }

            bool Connected() {
                if (sock->isOpen())
                    return true;
                else {
                    sock->open();
                    return sock->isOpen();
                }
            }
        private:
            int32_t id;
            std::string host;
            boost::shared_ptr<apache::thrift::transport::TSocket> sock;
            std::shared_ptr<raftfs::protocol::RaftServiceClient> rpc_client;

        };

        class RaftConsensus {
        public:

            enum class Role {
                kFollower,
                kCandidate,
                kLeader
            };

            RaftConsensus(Options opt);
            int64_t GetTerm() const { return current_term; }

            void StartRemoteLoops();
            void StartLeaderCheckLoop();
            
            void OnRequestVote(protocol::ReqVoteResponse& resp, const protocol::ReqVoteRequest& req);
            void OnAppendEntries(protocol::AppendEntriesResponse& resp, const protocol::AppendEntriesRequest& req);
        private:

            int64_t GetLastLogTerm() const {return log.back().term;}
            int64_t GetLastLogIndex() const {return log.back().index;}
            /*
             * Perodically check whether the leader node is timeout.
             * Should run in a thread
             */
            void CheckLeaderLoop();
            /*
             * Run this function in a thread for every remote node
             * perform operations based on current role
             */
            void RemoteHostLoop(std::shared_ptr<RemoteHost> remote);
            void StartLeaderElection();
            void ChangeToLeader();
            void PostponeElection();

        private:
            int32_t  self_id;
            std::string self_name;
            // a mapping from node id to its instance
            // may wrape it in a sperate class later
            std::map<int32_t, std::shared_ptr<RemoteHost>> remotes;

            std::atomic_bool stop;

            int64_t current_term;

            // for each term, we store the host id this node vote for
            //                 term     vote id
            std::unordered_map<int64_t, int32_t> vote_for;

            /*
             * for each election round, vote_pool stores the remote node id who grant their votes
             * need to be cleared at the beginning of election
             */
            std::set<int32_t> vote_pool;
            /*
             * The minimum number of responses for any valid operation
             * total number of nodes in the cluster X = 2*N + 1
             * where N is quorum size
             */
            int32_t quorum_size;
            int32_t leader_id;
            Role current_role;
            /*
             * time point to start a new election
             *
             */
            std::chrono::steady_clock::time_point next_election;

            // temporary in memory log
            // TODO implement a log class
            std::list<raftfs::protocol::Entry> log;

            std::mutex m;
            std::condition_variable new_event;
        };
    }
}



#endif //RAFTFS_RAFTCONSENSUS_H
