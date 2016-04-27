//
// Created by yzhh on 4/10/16.
//

#include <iostream>
#include "RaftFS.h"
#include "../utils/Options.h"



using namespace std;
using namespace raftfs;
using namespace raftfs::client;

int main(int argc, char** argv) {

    Options opt(argc, argv);
    FSClient client(opt);

    //client.CheckLeaders();

    string path("/test");
    for (int i = 0; i < 3; ++i) {
        string p = path + to_string(i);
        cout << "mkdir " << p << client.Mkdir(p) << endl;

    }
    cout << "hello world" << endl;

    return 0;
}
