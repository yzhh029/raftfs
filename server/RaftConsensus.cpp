//
// Created by yzhh on 3/30/16.
//

#include "RaftConsensus.h"
#include <iostream>

using namespace std;

namespace raftfs {
    namespace server {

        RaftConsensus::RaftConsensus()
            : currentTerm(11) {
            cout << " consensus init " << endl;
        }


    }
}