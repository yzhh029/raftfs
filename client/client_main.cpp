//
// Created by yzhh on 4/10/16.
//

#include <iostream>
#include "RaftFS.h"
#include "../utils/Options.h"

using namespace std;
using namespace raftfs;

int main(int argc, char** argv) {

    Options opt(argc, argv);
    FSClient client(opt);

    client.CheckLeaders();
    cout << "hello world" << endl;

    return 0;
}