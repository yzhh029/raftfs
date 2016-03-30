//
// Created by yzhh on 3/29/16.
//

#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <protocol/TMultiplexedProtocol.h>
#include <processor/TMultiplexedProcessor.h>

#include "MetaRPCServer.h"
#include "RaftService.h"
#include "RaftRPCService.h"
//#include <memory>
#include <iostream>
#include <boost/shared_ptr.hpp>


using namespace boost;
using namespace TH;
using namespace TH::server;
using namespace std;


namespace raftfs {

    namespace server {

        MetaRPCServer::MetaRPCServer(int _port, std::shared_ptr<RaftConsensus> _raft_state)
            : port(_port), raft_state(_raft_state) {

            // RaftRPCService
            boost::shared_ptr<TProcessor> raftProcessor(
                    new protocol::RaftServiceProcessor(
                            boost::shared_ptr<server::RaftRPCService>(new server::RaftRPCService)
                    ));

            boost::shared_ptr<TMultiplexedProcessor> processor(new TMultiplexedProcessor);

            processor->registerProcessor("RaftService", raftProcessor);

            boost::shared_ptr<TServerTransport> serverTransport(new transport::TServerSocket(port));
            boost::shared_ptr<TTransportFactory> transportFactory(new transport::TBufferedTransportFactory());
            boost::shared_ptr<TProtocolFactory> protocolFactory(new TH::protocol::TBinaryProtocolFactory());

            rpc_server.reset(new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory));

            cout << raft_state->GetTerm() << endl;
        }

        MetaRPCServer::~MetaRPCServer() {

        }


    }
}