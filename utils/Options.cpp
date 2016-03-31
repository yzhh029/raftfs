//
// Created by yzhh on 3/31/16.
//

#include "Options.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <unistd.h>

using namespace std;

namespace raftfs {


    Options::Options(int argc, char **argv) {

        if (argc == 3 && strcmp(argv[1], "-f") == 0) {
            host_file = argv[2];
        } else {
            cout << "Usage: ./raftfs -f hosts.txt" << endl;
            exit(1);
        }
    }


    std::string Options::GetSelfName() {

        char name[1024];
        gethostname(name, 1024);
        self_name = string(name);

        return self_name;
    }


    std::vector<std::string> Options::GetHosts() {
        ifstream infile(host_file);

        if (self_name.empty()) {
            GetSelfName();
        }

        vector<string> hosts;
        string h;
        bool found_self = false;
        while (infile >> h) {
            if (h == self_name)
                found_self = true;
            hosts.push_back(h);
        }

        if (!found_self) {
            cout << self_name << " is not in the hosts file" << endl;
            exit(1);
        }

        return hosts;
    }


}