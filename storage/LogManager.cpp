/*
 * RaftLog.h
 * - References: Logcabin: https://github.com/logcabin/
 *
 *  Created on: Apr 11, 2016
 *      Author: huang630
 */

#include "LogManager.h"
#include "../filesystem/FSNamespace.h"
#include <iostream>
#include <mutex>
#include <algorithm>
#include <iomanip>

using namespace std;
using namespace raftfs::filesystem;

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
        //
        bool LogManager::Append(const std::vector<Entry> * p_new_entries) {
            std::lock_guard<std::mutex> guard(m);

            // if local is a empty log
            // or if the first entry's index is immediately after last local entry's index
            // simply append all new log entries to the end of local log
            if (memory_log.empty() || p_new_entries->front().index == memory_log.back()->index + 1) {
                Entry* copy = nullptr;
                for (auto& e : *p_new_entries) {
                    copy = new Entry(e);
                    memory_log.push_back(copy);
                }
                return true;
            }

            // when the first new entry index < local last index
            // find the first same entry in local log
            auto first_same_it = find_if(memory_log.begin(), memory_log.end(),
                                         [&p_new_entries](const Entry* e) {
                                             return p_new_entries->front().index == e->index
                                                    && p_new_entries->front().term == e->term;
                                         });
            // found the same entry in local log
            if (first_same_it != memory_log.end()) {
                cout << "found first same entry T:" << (*first_same_it)->term << " I:" << (*first_same_it)->index << endl;
                for (auto it = p_new_entries->begin(); it != p_new_entries->end(); ++it) {
                    // need to overwrite local log entris
                    if (first_same_it < memory_log.end()) {

                        // if the index and term of two entires are same, then they have the same cmd
                        if (it->index == (*first_same_it)->index && it->term == (*first_same_it)->term) {
                            ++first_same_it;
                            continue;
                        }
                        cout << " conflict entry local(" << (*first_same_it)->term << "," << (*first_same_it)->index
                        << ") leader(" << it->term << "," << it->index << ")" << endl;

                        // delete locol conflict log entry (*first_same_it)
                        delete *first_same_it;
                        *first_same_it = new Entry(*it);
                        ++first_same_it;
                    } else {
                        // more new log
                        Entry* copy = new Entry(*it);
                        memory_log.push_back(copy);
                    }
                }
                // delete extra conflict entries
                if (first_same_it != memory_log.end()) {
                    for (auto tmp_it = first_same_it; tmp_it != memory_log.end(); ++ tmp_it) {
                        delete *tmp_it;
                    }
                    memory_log.erase(first_same_it, memory_log.end());
                }
                return true;
            } else {
                // not found first new entry in existing log
                cout << "no same entry found, reject ae" << endl;
                return false;
            }
        }


        const LogManager::Entry *LogManager::GetEntry(int64_t index) const {

            auto it = find_if(memory_log.rbegin(), memory_log.rend(), [index](const Entry* e){
                return e->index == index;
            });

            if (it != memory_log.rend()){
                return *it;
            } else {
                return nullptr;
            }

        }


        vector<LogManager::Entry> LogManager::GetEntriesStartAt(int64_t start_index) const {

            auto it = lower_bound(memory_log.begin(), memory_log.end(), start_index, [](const Entry* lhs, int64_t index){ return lhs->index <= index;});
            vector<Entry> entries;
            for (; it != memory_log.end() ; ++it) {
                //cout << "pack " << (*it)->index << endl;
                entries.push_back(*(*it));
            }
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
            if (new_index <= memory_log.back()->index)
                last_commited_index = new_index;
            else
                cout << " wrong commit index " << new_index << " LI:" << memory_log.back()->index << endl;
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


        std::ostream &operator<<(std::ostream &os, const LogManager &lm) {
            auto it = lm.memory_log.begin();
            if (lm.memory_log.back()->index > 10)
                it = lm.memory_log.end() - 10;
            for (; it != lm.memory_log.end(); ++it) {
                cout << (*it)->term << " " << (*it)->index << " " << OpToStr((*it)->op) << " " << (*it)->value << endl;
            }
            return os;
        }

    } // namespace raftfs::server
} // namespace raftfs
