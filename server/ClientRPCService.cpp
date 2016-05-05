//
// Created by yzhh on 3/29/16.
// - GetLeader: For a client to get current leader.
//

#include "ClientRPCService.h"
#include "../filesystem/FSNamespace.h"

#include <iostream>

using namespace std;
using namespace raftfs::protocol;
using namespace raftfs::filesystem;

#define ENABLE_DBG_OUTPUT 0

namespace raftfs {
    namespace server {

        void ClientRPCService::GetLeader(GetLeaderResponse &_return) {

            _return.leader_id = raft_state.GetLeader();
            cout << " recv GET LEADER " << _return.leader_id << endl;
        }


        void ClientRPCService::Mkdir(MkdirResponse &_return, const MkdirRequest &new_dir) {

            if (!FSNamespace::ValidatePath(new_dir.path)) {
                _return.status = Status::kPathError;
                return;
            }

            if (!raft_state.IsLeader()) {
                _return.status = Status::kNotLeader;
                _return.__set_leader_id(raft_state.GetLeader());
                return ;
            }
			#if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv MKDIR " << new_dir.path << endl;
			#endif
            _return.status = raft_state.OnMetaOperation(MetaOp::kMkdir, new_dir.path, nullptr);
        }


        void ClientRPCService::Rmdir(protocol::RmdirResponse &_return, const protocol::RmdirRequest &dir) {
            if (!FSNamespace::ValidatePath(dir.dir)) {
                _return.status = Status::kPathError;
                return;
            }

            if (!raft_state.IsLeader()) {
                _return.status = Status::kNotLeader;
                _return.__set_leader_id(raft_state.GetLeader());
                return ;
            }
			#if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv RMDIR " << dir.dir << endl;
			#endif
            _return.status = raft_state.OnMetaOperation(MetaOp::kRmdir, dir.dir, nullptr);
        }


        void ClientRPCService::CreateFile(protocol::CreateFileResponse &_return,
                                          const protocol::CreateFileRequest &new_file) {

            if (!FSNamespace::ValidatePath(new_file.file_name)) {
                _return.status = Status::kPathError;
                return;
            }

            if (!raft_state.IsLeader()) {
                _return.status = Status::kNotLeader;
                _return.__set_leader_id(raft_state.GetLeader());
                return ;
            }
			#if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv CREATEFILE " << new_file.file_name << endl;
			#endif
            _return.status = raft_state.OnMetaOperation( MetaOp::kCreate, new_file.file_name, nullptr);
        }


        void ClientRPCService::InjectTestCase(protocol::TestCaseResponse& _return, const protocol::TestCaseRequest& req) {
            // TODO: call test case functions.
        	cout << " Inject test case: " << req.test_type << endl;
            //raft_state.OnMetaOperation(MetaOp::kMkdir, new_dir.path, nullptr);
        }


        void ClientRPCService::DeleteFile(protocol::DeleteResponse &_return, const protocol::DeleteRequest &path) {
            if (!FSNamespace::ValidatePath(path.path)) {
                _return.status = Status::kPathError;
                return;
            }

            if (!raft_state.IsLeader()) {
                _return.status = Status::kNotLeader;
                _return.__set_leader_id(raft_state.GetLeader());
                return ;
            }
			#if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv DELETE " << path.path << endl;
			#endif
            _return.status = raft_state.OnMetaOperation( MetaOp::kDelete, path.path, nullptr);
        }

        void ClientRPCService::GetFileInfo(protocol::FileInfoResponse &_return, const protocol::FileInfoRequest &file) {

            if (!FSNamespace::ValidatePath(file.file)) {
                _return.status = Status::kPathError;
                return;
            }
			#if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv GETFILEINFO " << file.file << endl;
			#endif
            if (!fs.GetFileInfo(file.file, _return.info)) {
                _return.status = Status::kNotFound;
                return;
            }
            _return.status = Status::kOK;
        }

        void ClientRPCService::ListDir(protocol::ListDirResponse &_return, const protocol::ListDirRequest &dir) {

            if (!FSNamespace::ValidatePath(dir.dir)) {
                _return.status = Status::kPathError;
                return;
            }

            #if(ENABLE_DBG_OUTPUT == 1)
            cout << " recv LISTDIR " << dir.dir << endl;
			#endif

            _return.__set_dir_list(fs.ListDir(dir.dir));

            #if(ENABLE_DBG_OUTPUT == 1)
            for (auto& l : _return.dir_list) {
                cout << l << endl;
            }
            cout << " reply ok" << endl;
			#endif
            _return.status = Status::kOK;

        }


    }
}
