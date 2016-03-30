//
// Created by yzhh on 3/29/16.
//

#include <iostream>
#include <thread>
#include <boost/program_options.hpp>
#include "RaftMetaServer.h"

using namespace std;
using namespace raftfs::server;

int main() {

    RaftMetaServer server;
    server.run();
    cout << "Hello, World!" << endl;
    return 0;
}