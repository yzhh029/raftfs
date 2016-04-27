//
// Created by yzhh on 3/31/16.
// - Host must includes ourself.
//

#include "Options.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <unistd.h>

using namespace std;

namespace raftfs {


    Options::Options(int argc, char **argv) {

        if (argc == 5 && strcmp(argv[1], "-f") == 0 && strcmp(argv[3], "-p") == 0) {
            host_file = argv[2];
            port = atoi(argv[4]);
        } else {
            cout << "Usage: ./raftfs -f hosts.txt -p port" << endl;
            exit(1);
        }
    }


    std::string Options::GetSelfName() {

        if (self_name.empty()) {
            char name[1024];
            gethostname(name, 1024);
            self_name = string(name);
        }

        return self_name;
    }


    map<int32_t, string> Options::GetHosts() {
        ifstream infile(host_file);

        int id = 1;

        string h;
        bool found_self = false;
        while (infile >> h) {
            if (h == self_name) {
                found_self = true;
                self_id = id++;
            } else {
                hosts[id++] = h;
            }
        }

        if (!found_self) {
            cout << self_name << " is not in the hosts file" << endl;
            exit(1);
        }

        return hosts;
    }


    std::vector<std::string> Options::GetAllHosts() {

        if (!all_hosts.empty())
            return all_hosts;

        ifstream infile(host_file);
        string h;

        while (infile >> h) {
            all_hosts.push_back(h);
        }

        return all_hosts;
    }


}
