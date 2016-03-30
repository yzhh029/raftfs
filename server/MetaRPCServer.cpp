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
#include <boost/shared_ptr.hpp>


using namespace boost;
using namespace TH;
using namespace TH::server;

namespace raftfs {

    namespace server {

        MetaRPCServer::MetaRPCServer(int _port)
            : port(_port) {

            // RaftRPCService
            shared_ptr<TProcessor> raftProcessor(
                    new protocol::RaftServiceProcessor(
                            shared_ptr<server::RaftRPCService>(new server::RaftRPCService)
                    ));

            shared_ptr<TMultiplexedProcessor> processor(new TMultiplexedProcessor);

            processor->registerProcessor("RaftService", raftProcessor);

            shared_ptr<TServerTransport> serverTransport(new transport::TServerSocket(port));
            shared_ptr<TTransportFactory> transportFactory(new transport::TBufferedTransportFactory());
            shared_ptr<TProtocolFactory> protocolFactory(new TH::protocol::TBinaryProtocolFactory());

            rpc_server.reset(new TSimpleServer(processor, serverTransport, transportFactory, protocolFactory));

        }

        MetaRPCServer::~MetaRPCServer() {

        }


    }
}