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
#include <transport/TSocket.h>

#include "../protocol/RaftService.h"
#include "../utils/Options.h"


namespace raftfs {
    namespace server {

        class RemoteHost {

        public:
            RemoteHost(std::string host_name, int port);
            ~RemoteHost() {
                if (sock->isOpen())
                    sock->close();
            }
            std::string GetName() const {return host;}
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
            int64_t GetTerm() const {return current_term;}

            void StartRemoteLoops();
            void StartLeaderCheckLoop();
        private:
            void CheckLeaderLoop();
            void RemoteHostLoop(std::shared_ptr<RemoteHost> remote);
            void StartLeaderElection();

        private:
            int64_t  self_id;
            std::string self_name;
            std::map<int64_t, std::shared_ptr<RemoteHost>> remotes;

            std::atomic_bool stop;

            int64_t current_term;
            std::string vote_for;
            int64_t leader_id;
            Role current_role;
            std::chrono::steady_clock::time_point next_election;


            std::mutex m;
            std::condition_variable new_event;
            //std::vector<std::thread> peer_threads;
        };
    }
}



#endif //RAFTFS_RAFTCONSENSUS_H
