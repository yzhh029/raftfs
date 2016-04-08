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


namespace raftfs {
    namespace server {

        class RemoteHost {

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

            void CheckLeaderLoop();
            void RemoteHostLoop(std::shared_ptr<RemoteHost> remote);
            void StartLeaderElection();
            void ChangeToLeader();

        private:
            int32_t  self_id;
            std::string self_name;
            std::map<int32_t, std::shared_ptr<RemoteHost>> remotes;

            std::atomic_bool stop;

            int64_t current_term;
            // for each term, we store the host id this node vote for
            //                 term     vote id
            std::unordered_map<int64_t, int32_t> vote_for;
            std::set<int32_t> vote_pool;
            int32_t quorum_size;
            int32_t leader_id;
            Role current_role;
            std::chrono::steady_clock::time_point next_election;

            // temporary in memory log
            std::list<raftfs::protocol::Entry> log;

            std::mutex m;
            std::condition_variable new_event;
            //std::vector<std::thread> peer_threads;
        };
    }
}



#endif //RAFTFS_RAFTCONSENSUS_H
