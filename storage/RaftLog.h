/*
 * RaftLog.h
 * - References: Logcabin: https://github.com/logcabin/
 *
 *  Created on: Apr 8, 2016
 *      Author: huang630
 */

#ifndef RAFTLOG_H_
#define RAFTLOG_H_

#include "../protocol/Raft_types.h"
#include "../server/RaftConsensus.h"
#include "../protocol/RaftService.h"
//#include <memory>
#include <vector>

#undef SYNC_ENABLE

typedef raftfs::protocol::Entry Entry; // for short.

namespace raftfs {
    namespace  server {		// FIXME: Temporary put this under server...
        class RaftLog {

        public:
        	RaftLog();
            virtual ~RaftLog();

            // Data structures to entry storage
			std::vector<const Entry*> memoryLog;
			Entry empty_ent;

			/* Append Entry(s) and return range of indexes of the new entries
			 * in the log, inclusive.
			 */
			std::pair<uint64_t, uint64_t> append(
					const std::vector<const Entry*>& entries);


			const Entry& getEntry(uint64_t index) const;

			/* Append Entry(s) and return range of indexes of the new entries
			 * in the log, inclusive.
			 */
			uint64_t getLogStartIndex() const;
			uint64_t getLastLogIndex() const;

		#ifdef SYNC_ENABLE
			virtual std::string getName() const = 0;
			virtual uint64_t getSizeBytes() const = 0;
		#endif


			// TODO: If we need a sync function, add or modify these...
		#ifdef SYNC_ENABLE
			void syncComplete( /*std::unique_ptr<FlushStorage> sync) {
				sync->completed = true;
				syncCompleteVirtual(std::move(sync));
			}*/
				);
			virtual void syncCompleteVirtual(/*std::unique_ptr<FlushStorage> sync*/) {
			}
			virtual std::unique_ptr<FlushStorage> takeSync() = 0;
		#endif

		protected:


		public:


			/**
			 * Remove log entries before the given index.
			 */
			void removeEntryBefore(uint64_t firstIndex);

			/**
			 * Delete log entries past the given index.
			 */
			void removeEntryAfter(uint64_t lastIndex);

		#ifdef SYNC_ENABLE
			/**
			 * Call this after changing #metadata.
			 */
			//virtual void updateMetadata() = 0;
			/**
			 * Add information about the log's state to the given structure.
			 * Used for diagnostics.
			 */
			//virtual void updateServerStats(Protocol::ServerStats& serverStats) const {}
			/**
			 * Opaque metadata that the log keeps track of.
			 */
			//Protocol::RaftLogMetadata::Metadata metadata;
		#endif

			// Print a Log for debugging purposes.
			friend std::ostream& operator<<(std::ostream& os, const RaftLog& log);

        protected:

			// Log is not copyable for saftty.
			RaftLog(const RaftLog&) = delete;
			RaftLog& operator=(const RaftLog&) = delete;



        private:
            //std::shared_ptr<RaftConsensus>& raft_state;
        };
    }
}




#endif /* RAFTLOG_H_ */
