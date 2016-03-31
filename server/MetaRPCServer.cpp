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
            rpc_server.reset(new TSimpleServer(
                    boost::make_shared<RaftServiceProcessor>(boost::make_shared<RaftRPCService>()),
                    boost::make_shared<TH::transport::TServerSocket>(_port),
                    boost::make_shared<TH::transport::TBufferedTransportFactory>(),
                    boost::make_shared<TH::protocol::TBinaryProtocolFactory>()
            ));

            cout << raft_state->GetTerm() << endl;
        }

        MetaRPCServer::~MetaRPCServer() {

        }


        void MetaRPCServer::run() {
            cout << "RPCserver start" << endl;
            rpc_server->serve();
        }


    }
}