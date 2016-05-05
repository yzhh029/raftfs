//
// Created by yzhh on 4/10/16.
// - File System Client
//

#include "RaftFS.h"
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TMultiplexedProtocol.h>

#include <chrono>

using namespace std;
using namespace raftfs::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift;
using namespace apache::thrift::protocol;

namespace raftfs {
    namespace client {
        FSClient::FSClient(Options &opt)
                : port(opt.GetPort()),
                  hosts(opt.GetAllHosts()),
                  leader_id(-1)
                  //follower_id((rand() % hosts.size()) + 1)
        {
        	//std::srand(time(0));	// don't know why xinu11 has problem with this... follower_id is strange...
        	follower_id = (rand() % hosts.size()) + 1;
            cout << " try to connect to " << follower_id << " " + hosts[follower_id - 1] << endl;
            ResetRPCClient(follower_rpc, follower_sock, hosts[follower_id - 1]);

            cout << "ask " << follower_sock->getHost() << endl;
            // get current leader
            GetLeader();
            if (leader_id != -1)
                cout << "current leader is" << hosts[leader_id - 1] << endl;
            else
                cout << "no leader" << endl;
        }

        FSClient::~FSClient() {
            if (follower_sock != nullptr && follower_sock->isOpen())
                follower_sock->close();

            if (leader_sock != nullptr && leader_sock->isOpen())
                leader_sock->close();
        }

        bool FSClient::ConnectFollower() {
            if (!follower_sock || !follower_sock->isOpen()) {
                int retry = 0;
                while (!ResetRPCClient(follower_rpc, follower_sock, hosts[follower_id - 1])
                       && retry < hosts.size()) {
                    follower_id = (follower_id + 1) % hosts.size() + 1;
                    ++retry;
                }
            }

            return follower_sock->isOpen();
        }

        int32_t FSClient::GetLeader() {
            if (leader_id == -1 && ConnectFollower()) {

                protocol::GetLeaderResponse resp;

                cout << " connected to " << follower_sock->getHost() << endl;

                follower_rpc->GetLeader(resp);
                if (resp.status == Status::kOK) {
                    leader_id = resp.leader_id;
                    cout << " new leader is " << leader_id << " " << hosts[leader_id - 1] << endl;
                    ResetRPCClient(leader_rpc, leader_sock, hosts[leader_id - 1]);

                    // if current follower and leader are the same host
                    // change follower to another host
                    if (leader_id == follower_id) {
                        follower_id = (follower_id + 1) % hosts.size() + 1;
                        ResetRPCClient(follower_rpc, follower_sock, hosts[follower_id - 1]);
                    }
                } else {
                    cout << "failed to get leader id " << resp.status << endl;
                }
            }
            return leader_id;
        }

        void FSClient::CheckLeaders() {
            for (auto &h : hosts) {
                ResetRPCClient(follower_rpc, follower_sock, h);
                cout << " ask " << h << endl;
                leader_id = -1;
                GetLeader();
            }
        }

        Status_ FSClient::Mkdir(const string &abs_dir) {

            //auto start = chrono::system_clock::now();
            if (GetLeader() == -1) {
                return Status::kNoLeader;
            }
            if (!leader_rpc) {
                ResetRPCClient(leader_rpc, leader_sock, hosts[leader_id - 1]);
            }
            //cout << "do mkdir " << endl;
            MkdirRequest req;
            req.path = abs_dir;
            MkdirResponse resp;
            //cout << leader_sock->getHost() << " " << hosts[leader_id - 1] << endl;
            try {
                leader_rpc->Mkdir(resp, req);
            } catch (TTransportException e) {
                return Status::kCommError;
            }

            //auto end = chrono::system_clock::now();
            //cout << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;
            return resp.status;
        }

        Status_ FSClient::ListDir(const std::string &abs_dir, std::vector<std::string> &dir_list) {

            if (!ConnectFollower())
                return Status::kCommError;

            ListDirRequest req;
            ListDirResponse resp;

            req.dir = abs_dir;
            try {
                follower_rpc->ListDir(resp, req);
            } catch (TTransportException e) {
                return Status::kCommError;
            }

            if (resp.status == Status::kNotFound) {
                resp = ListDirResponse();
                try {
                    leader_rpc->ListDir(resp, req);
                } catch (TTransportException e) {
                    return Status::kCommError;
                }

            }
            dir_list = resp.dir_list;
            return resp.status;
        }

        Status_ FSClient::GetFileInfo(const std::string &file_path, protocol::FileInfo &file_info) {
            if (!ConnectFollower())
                return Status::kCommError;

            FileInfoRequest req;
            FileInfoResponse resp;

            req.file = file_path;
            try {
                follower_rpc->GetFileInfo(resp, req);
            } catch (TTransportException e) {
                return Status::kCommError;
            }

            if (resp.status == Status::kOK) {
                file_info = resp.info;
            }
            return resp.status;
        }

        Status_ FSClient::CreateFile(const std::string &file_path) {

            if (GetLeader() == -1) {
                return Status::kNoLeader;
            }

            CreateFileRequest req;
            CreateFileResponse resp;

            req.file_name = file_path;
            try {
                leader_rpc->CreateFile(resp, req);
            } catch (TTransportException e) {
                return Status::kCommError;
            }

            return resp.status;
        }

        Status_ FSClient::Delete(const std::string &path) {

            if (GetLeader() == -1) {
                return Status::kNoLeader;
            }

            DeleteRequest req;
            DeleteResponse resp;

            req.path = path;
            try {
                leader_rpc->Delete(resp, req);
            } catch (TTransportException e) {
                return Status::kCommError;
            }

            if (resp.status == Status::kOK && resp.__isset.leader_id && resp.leader_id != leader_id) {
                leader_id = resp.leader_id;
                ResetRPCClient(leader_rpc, leader_sock, hosts[leader_id - 1]);
            }

            return resp.status;
        }


        void FSClient::ResetSock(boost::shared_ptr<THt::TSocket> &sock, string host) {
            if (sock != nullptr && sock->isOpen())
                sock->close();
            sock.reset(new TSocket(host, port));
        }

        bool FSClient::ResetRPCClient(std::shared_ptr<protocol::ClientServiceClient> &client,
                                      boost::shared_ptr<THt::TSocket> &sock,
                                      string host) {
            ResetSock(sock, host);
            //boost::shared_ptr<TBinaryProtocol> proto(new TBinaryProtocol(follower_sock));
            boost::shared_ptr<TCompactProtocol> proto(new TCompactProtocol(sock));
            boost::shared_ptr<TMultiplexedProtocol> client_proto(
                    new TMultiplexedProtocol(proto, "FSClient")
            );

            client.reset(new raftfs::protocol::ClientServiceClient(client_proto));
            try {
                sock->open();
            } catch (TTransportException e) {
                cout << "open connection to " << sock->getHost() << " failed" << endl;
                return false;
            }
            return true;
        }

    }
}
