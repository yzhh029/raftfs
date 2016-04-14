/*
 * RaftLog.h
 * - References: Logcabin: https://github.com/logcabin/
 *
 *  Created on: Apr 11, 2016
 *      Author: huang630
 */

#include "LogManager.h"
#include <mutex>

using namespace std;

namespace raftfs {
    namespace server {    // FIXME: put this under server first

        LogManager::LogManager()
                : last_commited_index(0) {

        }

        LogManager::~LogManager() {
            // TODO: check if we need to flush before closed.
        }


        bool LogManager::IsEmpty() const {
            std::lock_guard<std::mutex> guard(m);
            return memory_log.empty();
        }


        int64_t LogManager::Size() const {
            lock_guard<mutex> guard(m);
            return memory_log.size();
        }


        bool LogManager::Append(LogManager::Entry *new_entry) {
            std::lock_guard<std::mutex> guard(m);
            if (memory_log.empty() ||
                    (new_entry->term >= memory_log.back()->term
                     && new_entry->index > memory_log.back()->index
                    )
                ) {
                memory_log.push_back(new_entry);
                return true;
            }
            return false;
        }

        bool LogManager::Append(std::vector<Entry *> new_entries) {
            return false;
        }

        int64_t LogManager::GetLastLogIndex() const {
            lock_guard<mutex> lock(m);
            if (memory_log.empty()) {
                return 0;
            }
            return memory_log.back()->index;
        }


        int64_t LogManager::GetLastCommitIndex() const {
            lock_guard<mutex> lock(m);
            if (memory_log.empty()) {
                return 0;
            }
            return last_commited_index;
        }


        int64_t LogManager::GetLogStartIndex() const {
            lock_guard<mutex> lock(m);
            if (memory_log.empty()) {
                return 0;
            }
            return memory_log.back()->index;
        }

        int64_t LogManager::GetLastLogTerm() const {
            lock_guard<mutex> lock(m);
            if (memory_log.empty()) {
                return 0;
            }
            return memory_log.back()->term;
        }

        std::pair<int64_t, int64_t> LogManager::GetLastLogTermAndIndex() const {
            lock_guard<mutex> lock(m);

            if(memory_log.empty())
                return pair<int64_t, int64_t>(0, 0);
            return pair<int64_t, int64_t>(memory_log.back()->term, memory_log.back()->index);
        }




// TODO: Finish these function bodies.
        /*
std::pair<uint64_t, uint64_t>  LogManager::append(
                    const std::vector<const Entry*>& entries)
{
    // TODO: Fix by range or sorting by index.
    std::pair<uint64_t, uint64_t> rtn;
    uint64_t begin, end;
    if(entries.size() == 0) {
        begin = -1;
        end = -1;
    } else {
        begin = entries[0]->index;
        end = entries[entries.size()-1]->index;
        for(auto && p: entries) {
            memoryLog.push_back(p);
        }
    }
    rtn = std::make_pair(0,0);
    return rtn;
}


const Entry& LogManager::getEntry(uint64_t index) const {
    for(auto &&p: memory_log) {
        if(p->index == index) {
            return *p;
        }
    }
    return empty_ent;
}

uint64_t LogManager::getLogStartIndex() const {
    return memoryLog[0]->index;
}

uint64_t LogManager::getLastLogIndex() const {
    const Entry * ptr = memoryLog.back();
    return ptr->index;


}

void LogManager::removeEntryBefore(uint64_t firstIndex)
{	//TODO: Verification
    std::vector<const Entry*>::iterator from, to;
    from = memoryLog.begin();
    int i;
    for(i=0; i<memoryLog.size(); ++i) {
        if(memoryLog[i]->index == firstIndex) {
            to = memoryLog.begin() + i;
            break;
        }
    }
    // Stop removal if found nothing.
    if(i == memoryLog.size()) {
        cout << "Error: entry not found!" << endl;
        return;
    }
    memoryLog.erase(from, to);
}

void LogManager::removeEntryAfter(uint64_t firstIndex)
{	//TODO: Verification
    std::vector<const Entry*>::iterator from, to;
    int i;
    for(i=0; i<memoryLog.size(); ++i) {
        if(memoryLog[i]->index == firstIndex) {
            from = memoryLog.begin() + i + 1;
            to = memoryLog.end();
            break;
        }
    }
    // Stop removal if found nothing.
    if(i == memoryLog.size()) {
        cout << "Error: entry not found!" << endl;
        return;
    }
    memoryLog.erase(from, to);
}
*/

    } // namespace raftfs::server
} // namespace raftfs
