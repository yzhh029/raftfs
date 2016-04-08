//
// Created by yzhh on 4/8/16.
//

#ifndef RAFTFS_TIME_H
#define RAFTFS_TIME_H

#include <chrono>
#include <string>

inline std::chrono::steady_clock::time_point Now() {
    return std::chrono::steady_clock::now();
}

inline std::string TimePointStr(std::chrono::steady_clock::time_point tp) {
    std::time_t tp_time = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + (tp - std::chrono::steady_clock::now())
    );
    tm local_tm = *localtime(&tp_time);
    char buf[15];
    int size = strftime(buf, 15, "[%T]", &local_tm);
    return std::string(buf, size);
}


#endif //RAFTFS_TIME_H
