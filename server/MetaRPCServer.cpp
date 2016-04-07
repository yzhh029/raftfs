//
// Created by yzhh on 3/29/16.
//

#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TMultiplexedProtocol.h>
#include <processor/TMultiplexedProcessor.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PlatformThreadFactory.h>

#include "MetaRPCServer.h"
#include "RaftService.h"
#include "RaftRPCService.h"
#include <memory>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>


using namespace boost;
using namespace TH;
using namespace TH::server;
using namespace std;
using namespace raftfs::protocol;

namespace raftfs {

    namespace server {

        MetaRPCServer::MetaRPCServer(int _port, std::shared_ptr<RaftConsensus> _raft_state)
            : raft_state(_raft_state) {

            // RaftRPCService

            boost::shared_ptr<TMultiplexedProcessor> mux_processor(new TMultiplexedProcessor);
            boost::shared_ptr<TProcessor> raft_processor(
                    new RaftServiceProcessor(
                            boost::shared_ptr<RaftRPCService>(new RaftRPCService())
                    )
            );
            mux_processor->registerProcessor("Raft", raft_processor);

            const int workerCount = 5;

            boost::shared_ptr<concurrency::ThreadManager> threadManager =
             concurrency::ThreadManager::newSimpleThreadManager(workerCount);
            threadManager->threadFactory(boost::make_shared<concurrency::PlatformThreadFactory>());
            threadManager->start();                                                                                                                                                                                                                                              

            rpc_server = new TThreadPoolServer(
                    mux_processor,
                    boost::make_shared<TH::transport::TServerSocket>(_port),
                    boost::make_shared<TH::transport::TBufferedTransportFactory>(),
                    boost::make_shared<TH::protocol::TBinaryProtocolFactory>(),
                    threadManager
            );

            /*
            rpc_server = new TSimpleServer(
                    mux_processor,
                    boost::make_shared<TH::transport::TServerSocket>(_port),
                    boost::make_shared<TH::transport::TBufferedTransportFactory>(),
                    boost::make_shared<TH::protocol::TBinaryProtocolFactory>()
            );
             */

            cout << raft_state->GetTerm() << endl;
        }

        MetaRPCServer::~MetaRPCServer() {
            delete rpc_server;
        }


        void MetaRPCServer::run() {
            cout << "RPCserver start" << endl;
            //rpc_server->serve();
            //thread rpc_th(&TServer::serve, rpc_server.get());
            //std::thread(&rpc_server->serve(), )
            cout << "RPCserver started" << endl;
            //rpc_th.detach();
            //this_thread::sleep_for(chrono::seconds(1));

            thread rpc_th([this](){this->rpc_server->serve();});
            raft_state->StartRemoteLoops();
            
            //rpc_server->serve();
            rpc_th.join();
        }


    }
}