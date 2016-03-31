//
// Created by yzhh on 3/30/16.
//

#ifndef RAFTFS_RAFTCONSENSUS_H
#define RAFTFS_RAFTCONSENSUS_H

#include <stdint.h>
#include <string>

namespace raftfs {
    namespace server {
        class RaftConsensus {
        public:
            RaftConsensus();
            int64_t GetTerm() const {return currentTerm;}
        private:
            int64_t currentTerm;
            std::string vote_for;

        };
    }
}



#endif //RAFTFS_RAFTCONSENSUS_H
