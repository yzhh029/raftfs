/*
 * RaftLog.h
 *
 *
 *  Created on: Apr 8, 2016
 *      Author: huang630
 */

#ifndef RAFTLOG_H_
#define RAFTLOG_H_

#include "../protocol/RaftService.h"
#include <memory>
#include <vector>
#include <deque>
#include <mutex>
#include <iostream>


namespace raftfs {
    namespace  server {		// FIXME: Temporary put this under server...
        class LogManager {

        public:

            typedef raftfs::protocol::Entry Entry; // for short.

            LogManager();
            virtual ~LogManager();

            bool IsEmpty() const ;
            int64_t Size() const ;
            void Print() const ;

			/* Append Entry(s) and return whether append sucess.
			 */
            bool Append(Entry * new_entry);
            bool Append(const std::vector<Entry> * new_entries);


			const Entry* GetEntry(int64_t index) const;

            std::vector<Entry *> GetNewCommitedEntries(int64_t ) const;

            std::vector<Entry> GetEntriesStartAt(int64_t start_index) const;
            int64_t GetEntryLoc(int64_t lookup_index);

			int64_t GetLogStartIndex() const;
			int64_t GetLastLogIndex() const;
            int64_t GetLastLogTerm() const;
            int64_t GetLastCommitIndex() const;
            std::pair<int64_t, int64_t> GetLastLogTermAndIndex() const;

            void SetLastCommitIndex(int64_t new_index);

			/**
			 * Remove log entries before the given index.
			 */
			void RemoveEntryBefore(int64_t firstIndex);

			/**
			 * Delete log entries past the given index.
			 */
			void RemoveEntryAfter(int64_t lastIndex);

			// Print a Log for debugging purposes.
			friend std::ostream& operator<<(std::ostream& os, const LogManager& log);

			// Log is not copyable for safety.
			LogManager(const LogManager&) = delete;
			LogManager& operator=(const LogManager&) = delete;

        private:
			// Data structures to entry storage
			std::deque<Entry *> memory_log;
            mutable std::mutex m;

            int64_t last_commited_index;
        };


        std::ostream& operator<<(std::ostream& os, const LogManager& lm);
    }
}


#endif /* RAFTLOG_H_ */
