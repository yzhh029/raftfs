//
// Created by yzhh on 4/10/16.
//

#ifndef RAFTFS_RAFTFS_H
#define RAFTFS_RAFTFS_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <transport/TSocket.h>
#include "../protocol/ClientService.h"
#include "../utils/Options.h"

namespace THt = apache::thrift::transport;

namespace raftfs {
    namespace client {

        typedef protocol::Status::type Status_;

        class FSClient {
        public:
            FSClient(Options &opt);

            ~FSClient();

            // only for test purpose
            void CheckLeaders();

            Status_ Mkdir(const std::string &abs_dir);
            Status_ ListDir(const std::string &abs_dir, std::vector<std::string> &dir_list);
            Status_ GetFileInfo(const std::string &file_path, protocol::FileInfo &file_info);
            Status_ CreateFile(const std::string &file_path);
            Status_ Delete(const std::string &path);

        private:
            int32_t GetLeader();
            bool ConnectFollower();

            void ResetSock(boost::shared_ptr<THt::TSocket> &sock, std::string host);

            bool ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client,
                                boost::shared_ptr<THt::TSocket> &sock,
                                std::string host);

        private:
            int port;
            int32_t leader_id;
            int32_t follower_id;
            std::vector<std::string> hosts;

            boost::shared_ptr<THt::TSocket> leader_sock;
            boost::shared_ptr<THt::TSocket> follower_sock;
            std::shared_ptr<protocol::ClientServiceClient> leader_rpc;
            std::shared_ptr<protocol::ClientServiceClient> follower_rpc;

        };
    }
}

#endif //RAFTFS_RAFTFS_H
