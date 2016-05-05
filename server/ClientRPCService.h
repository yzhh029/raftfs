//
// Created by yzhh on 3/29/16.
//

#ifndef RAFTFS_CLIENTRPCSERVICE_H
#define RAFTFS_CLIENTRPCSERVICE_H

#include "RaftConsensus.h"
#include "../protocol/ClientService.h"
#include "../filesystem/FSNamespace.h"
#include <memory>

namespace raftfs {
    namespace server {
        class ClientRPCService : virtual public protocol::ClientServiceIf {
        public:
            ClientRPCService(RaftConsensus& state, filesystem::FSNamespace& _fs)
                    : raft_state(state),
                      fs(_fs)
            {}

            void GetLeader(protocol::GetLeaderResponse &_return) override ;

            void Mkdir(protocol::MkdirResponse &_return, const protocol::MkdirRequest &new_dir) override;

            void Rmdir(protocol::RmdirResponse &_return, const protocol::RmdirRequest &dir) override;

            void CreateFile(protocol::CreateFileResponse &_return,
                                    const protocol::CreateFileRequest &new_file) override;

            void DeleteFile(protocol::DeleteResponse &_return, const protocol::DeleteRequest &path) override;

            void GetFileInfo(protocol::FileInfoResponse &_return,
                                     const protocol::FileInfoRequest &file) override;

            void ListDir(protocol::ListDirResponse &_return, const protocol::ListDirRequest &dir) override;

            void InjectTestCase(protocol::TestCaseResponse& _return, const protocol::TestCaseRequest& req) override;

        private:
            RaftConsensus& raft_state;
            filesystem::FSNamespace& fs;
        };
    }
}

#endif //RAFTFS_CLIENTRPCSERVICE_H
