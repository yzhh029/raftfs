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
#include "RaftConsensus.h"
#include "../protocol/RaftService.h"
//#include <memory>
#include <vector>

namespace raftfs {
    namespace  server {		// FIXME: Temporary put this under server...
        class RaftLog {

        public:
            RaftLog();

          /*************************************************
           * Flush log in memory to physical storage
           - Leader will call this in a thread.
           - Followers / Candidates call after AppendEntries()
           */
          class FlushStorage {
            public:
              explicit FlushStorage(uint64_t lastIndex);
              virtual ~FlushStorage();
              /**
               * Wait for the log entries to be durable.
               * This is safe to call while the Log is being accessed and modified
               * from a separate thread.
               */
              virtual void wait() {}
              /**
               * The index of the last log entry that is being flushed.
               * After the call to wait, every entry in the log up to this one is
               * durable.
               */
              uint64_t lastIndex;
              /**
               * Used by destructor to make sure that Log::syncComplete was called.
               */
              bool completed;
          };

          typedef raftfs::protocol::Entry Entry;	// for short.

          RaftLog();
          virtual ~RaftLog();

          /**
           * Start to append new entries to the log.
           * The entries will be in memory untio flush
           * \param entries
           *      Entries to place at the end of the log.
           * \return
           *      Range of indexes of the new entries in the log, inclusive.
           */
          virtual std::pair<uint64_t, uint64_t> append(
                                  const std::vector<const Entry*>& entries) = 0;

          /**
           * Look up an entry by its log index.
           */
          virtual const Entry& getEntry(uint64_t index) const = 0;

          /**
           * Get the index of the first entry in the log (whether or not this
           * entry exists).
           * \return
           *      1 for logs that have never had truncatePrefix called,
           *      otherwise the largest index passed to truncatePrefix.
           */
          virtual uint64_t getLogStartIndex() const = 0;

          /**
           * Get the index of the most recent entry in the log.
           * \return
           *      The index of the most recent entry in the log,
           *      or getLogStartIndex() - 1 if the log is empty.
           */
          virtual uint64_t getLastLogIndex() const = 0;

          /**
           * Return the name of the log implementation as it would be specified in
           * the config file.
           */
          virtual std::string getName() const = 0;

          /**
           * Get the size of the entire log in bytes.
           */
          virtual uint64_t getSizeBytes() const = 0;

          /**
           * Release resources attached to the Sync object.
           * Call this after waiting on the Sync object.
           */
          void syncComplete(std::unique_ptr<FlushStorage> sync) {
              sync->completed = true;
              syncCompleteVirtual(std::move(sync));
          }
        protected:
          /**
           * See syncComplete(). Intended for subclasses to override.
           */
          virtual void syncCompleteVirtual(std::unique_ptr<FlushStorage> sync) {}
        public:

          /**
           * Get and remove the Log's Sync object in order to wait on it.
           * This Sync object must later be returned to the Log with syncComplete().
           *
           * While takeSync() and syncComplete() may not be done concurrently with
           * other Log operations, Sync::wait() may be done concurrently with all
           * operations except truncateSuffix().
           */
          virtual std::unique_ptr<FlushStorage> takeSync() = 0;

          /**
           * Delete the log entries before the given index.
           * Once you truncate a prefix from the log, there's no way to undo this.
           * The entries may still be on disk when this returns and file descriptors
           * and other resources may remain open; see Sync.
           * \param firstIndex
           *      After this call, the log will contain no entries indexed less
           *      than firstIndex. This can be any log index, including 0 and those
           *      past the end of the log.
           */
          virtual void truncatePrefix(uint64_t firstIndex) = 0;

          /**
           * Delete the log entries past the given index.
           * This will not affect the log start index.
           * \param lastIndex
           *      After this call, the log will contain no entries indexed greater
           *      than lastIndex. This can be any log index, including 0 and those
           *      past the end of the log.
           * \warning
           *      Callers should wait() on all Sync object prior to calling
           *      truncateSuffix(). This never happens on leaders, so it's not a real
           *      limitation, but things may go wonky otherwise.
           */
          virtual void truncateSuffix(uint64_t lastIndex) = 0;

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

          /**
           * Print out a Log for debugging purposes.
           */
          friend std::ostream& operator<<(std::ostream& os, const Log& log);

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
