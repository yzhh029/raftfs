//
// Created by yzhh on 3/31/16.
//

#ifndef RAFTFS_OPTIONS_H
#define RAFTFS_OPTIONS_H

#include <string>
#include <vector>
#include <map>

namespace raftfs {
    class Options {
    public:
        Options(int argc, char* argv[]);
        std::map<int64_t, std::string> GetHosts() ;
        std::string GetSelfName();
        int64_t GetSelfId() {return self_id; }
        int GetPort() {return port;}

    private:
        std::string host_file;
        std::string self_name;
        int64_t self_id;
        std::map<int64_t, std::string> hosts;
        int port;
    };

}


#endif //RAFTFS_OPTIONS_H
