/*
 * RaftLog.h
 *
 *  Created on: Apr 8, 2016
 *      Author: huang630
 */

#ifndef RAFTLOG_H_
#define RAFTLOG_H_

#include "RaftConsensus.h"
#include "../protocol/RaftService.h"
#include <memory>

namespace raftfs {
    namespace  server {
        class RaftLog {
        public:
            RaftLog() :

            bool Insert() override;



        private:
            //std::shared_ptr<RaftConsensus>& raft_state;
        };
    }
}




#endif /* RAFTLOG_H_ */
