//
// Created by yzhh on 4/8/16.
//

#ifndef RAFTFS_TIME_H
#define RAFTFS_TIME_H

#include <chrono>

inline std::chrono::steady_clock::time_point Now() {
    return std::chrono::steady_clock::now();
}

#endif //RAFTFS_TIME_H
