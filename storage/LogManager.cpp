/*
 * RaftLog.h
 * - References: Logcabin: https://github.com/logcabin/
 *
 *  Created on: Apr 11, 2016
 *      Author: huang630
 */

#include "LogManager.h"
#include <mutex>
#include <algorithm>

using namespace std;

namespace raftfs {
    namespace server {    // FIXME: put this under server first

        LogManager::LogManager()
                : last_commited_index(0) {

        }

        LogManager::~LogManager() {
        	// TODO: shall we lock here?
            // TODO: check if we need to flush before closed.
        	for(auto p: memory_log) {
        		delete p;	// FIXME: remove this if don't need recycle entries.
        	}
        }


        bool LogManager::IsEmpty() const {
            std::lock_guard<std::mutex> guard(m);
            return memory_log.empty();
        }


        int64_t LogManager::Size() const {
            lock_guard<mutex> guard(m);
            return memory_log.size();
        }

        int64_t LogManager::GetEntryLoc(int64_t lookup_index) {
        	//lock_guard<mutex> guard(m);  //locked outside...
        	int i;
        	for(i=0; i < memory_log.size(); ++i) {
        		if(memory_log[i]->index == lookup_index) {
        			return i;
        		}
        	}
        	// Can't not found
        	return -1;
        }


        /* Receiver implementation #3 and #4: -> All done by LogManager.
         * #3: Delete existing entry if they conflict with leader.
         * #4: Append new entries not in the log
         *     --> One RPC can contain multiple entries...
         */
        bool LogManager::Append(LogManager::Entry *new_entry) {
        	// FIXME: These only contains Receiveer Implementation #4. Need #3.
        	// Currently not used...
            std::lock_guard<std::mutex> guard(m);

            new_entry->index = memory_log.size() + 1;
            memory_log.push_back(new_entry);

            return true;
        }

        /* Receiver implementation #3 and #4: -> All done by LogManager.
         * #3: Delete existing entry if they conflict with leader.
         * #4: Append new entries not in the log
         *     --> One RPC can contain multiple entries...
         */
        bool LogManager::Append(const std::vector<Entry> * p_new_entries) {
            std::lock_guard<std::mutex> guard(m);
            //
            if(p_new_entries->empty())
            	return false;

            // #4: Append new entries not in the log
            if (memory_log.empty() ||
                    (p_new_entries->front().term >= memory_log.back()->term
                     && p_new_entries->front().index > memory_log.back()->index
                    )
                ) {
            	// FIXME: Make a new copy of entries in request to
            	//        prevent request is deleted later after append
            	for (auto ent: *(p_new_entries)) {
            		Entry * ent_copy = new Entry(ent);
            		ent_copy->index = memory_log.size() + 1;
                    memory_log.push_back(ent_copy);
            	}
                return true;
            } else {
            	// #4
            	int64_t existing_entry_at;
            	existing_entry_at = GetEntryLoc(p_new_entries->front().index);
            	if(existing_entry_at == -1) {
            		// TODO: behavior needs to be dealt with...
            	} else {
            		// TODO: Need to check if we have deadlock inside...
            		RemoveEntryAfter(existing_entry_at);
            		for (auto ent: *(p_new_entries)) {
						Entry * ent_copy = new Entry(ent);
						ent_copy->index = memory_log.size() + 1;
						memory_log.push_back(ent_copy);
					}
            	}
            	return true;
            }
            // Some possible error path.
            return false;
        }


        vector<LogManager::Entry> LogManager::GetEntriesStartAt(int64_t start_index) const {

            auto it = lower_bound(memory_log.begin(), memory_log.end(), start_index, [](const Entry* lhs, int64_t index){ return lhs->index < index;});

            vector<Entry> entries;
            for (; it != memory_log.end() ; ++it)
                entries.push_back(*(*it));
            return entries;
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

        void LogManager::SetLastCommitIndex(int64_t new_index) {
            lock_guard<mutex> lock(m);
            //if (memory_log.empty()) {
            //    return 0;
            //}
            last_commited_index = new_index;
        }


        int64_t LogManager::GetLogStartIndex() const {
            lock_guard<mutex> lock(m);
            if (memory_log.empty()) {
                return 0;
            }
            //return memory_log.back()->index;
            return memory_log.front()->index;
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

        void LogManager::RemoveEntryAfter(int64_t firstIndex)	// inclusive
        {	//TODO: Verification
        	lock_guard<mutex> guard(m);
            std::deque<Entry *>::iterator from, to;
            int i;
            for(i=0; i<memory_log.size(); ++i) {
                if(memory_log[i]->index == firstIndex) {
                    from = memory_log.begin() + i;	// +i+1 if exclusive;
                    to = memory_log.end();
                    break;
                }
            }
            // Stop removal if found nothing.
            if(i == memory_log.size()) {
                cout << "Error: entry not found!" << endl;
                return;
            }
            memory_log.erase(from, to);
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
