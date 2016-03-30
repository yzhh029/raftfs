//
// Created by yzhh on 3/30/16.
//

#ifndef RAFTFS_RAFTCONSENSUS_H
#define RAFTFS_RAFTCONSENSUS_H

#include <stdint.h>

namespace raftfs {
    namespace server {
        class RaftConsensus {
        public:
            RaftConsensus();
        private:
            int64_t currentTerm;


        };
    }
}



#endif //RAFTFS_RAFTCONSENSUS_H
