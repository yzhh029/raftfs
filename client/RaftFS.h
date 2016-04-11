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
    class FSClient {
    public:
        FSClient(Options& opt);


    private:
        int32_t GetLeader();
        void ResetSock(boost::shared_ptr<THt::TSocket>& sock, std::string host);
        void ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient>& client, std::string host);

    private:
        int port;
        int32_t leader_id;
        std::vector<std::string> hosts;

        boost::shared_ptr<THt::TSocket> leader_sock;
        boost::shared_ptr<THt::TSocket> follower_sock;
        std::shared_ptr<protocol::ClientServiceClient> leader_rpc;
        std::shared_ptr<protocol::ClientServiceClient> follower_rpc;

    };
}

#endif //RAFTFS_RAFTFS_H
