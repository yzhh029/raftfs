//
// Created by yzhh on 3/31/16.
//

#ifndef RAFTFS_OPTIONS_H
#define RAFTFS_OPTIONS_H

#include <string>
#include <vector>

namespace raftfs {
    class Options {
    public:
        Options(int argc, char* argv[]);
        std::vector<std::string> GetHosts() ;
        std::string GetSelfName();

    private:
        std::string host_file;
        std::string self_name;
    };

}


#endif //RAFTFS_OPTIONS_H
