//
// Created by yzhh on 3/29/16.
//

#include "RaftMetaServer.h"
#include "RaftRPCService.h"
#include "ClientRPCService.h"
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TCompactProtocol.h>
#include <protocol/TMultiplexedProtocol.h>
#include <processor/TMultiplexedProcessor.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PlatformThreadFactory.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <thread>

using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::server;
using namespace std;
using namespace raftfs::protocol;

namespace raftfs {
    namespace server {

        RaftMetaServer::RaftMetaServer(Options &opt) :
                raft_state(new RaftConsensus(opt))
        {
            cout << "MetaServer init" << endl;
            InitRPCServer(opt.GetPort(), 10);
        }


        RaftMetaServer::~RaftMetaServer() {
            delete rpc_server;
        }

        void RaftMetaServer::InitRPCServer(int _port, int worker) {
            boost::shared_ptr<TMultiplexedProcessor> mux_processor(new TMultiplexedProcessor);
            boost::shared_ptr<TProcessor> raft_processor(
                    new RaftServiceProcessor(
                            boost::shared_ptr<RaftRPCService>(new RaftRPCService(*raft_state))
                    )
            );
            mux_processor->registerProcessor("Raft", raft_processor);

            boost::shared_ptr<TProcessor> client_processor(
                    new ClientServiceProcessor(
                            boost::shared_ptr<ClientRPCService>(new ClientRPCService(*raft_state))
                    )
            );
            mux_processor->registerProcessor("FSClient", client_processor);

            boost::shared_ptr<concurrency::ThreadManager> threadManager =
                    concurrency::ThreadManager::newSimpleThreadManager(worker);
            threadManager->threadFactory(boost::make_shared<concurrency::PlatformThreadFactory>());
            threadManager->start();

            rpc_server = new TThreadPoolServer(
                    mux_processor,
                    boost::make_shared<transport::TServerSocket>(_port),
                    boost::make_shared<transport::TBufferedTransportFactory>(),
                    //boost::make_shared<apache::thrift::protocol::TBinaryProtocolFactory>(),
                    boost::make_shared<apache::thrift::protocol::TCompactProtocolFactory>(),
                    threadManager
            );
        }


        void RaftMetaServer::Run() {
            cout << "FS server start" << endl;

            //thread rpc_thread([this](){this->rpc_server->serve();});
            raft_state->StartRemoteLoops();
            raft_state->StartLeaderCheckLoop();
            rpc_server->serve();
            //rpc_thread.join();
        }


    }
}


