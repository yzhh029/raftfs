/*
 * RaftLog.h
 * - References: Logcabin: https://github.com/logcabin/
 *
 *  Created on: Apr 11, 2016
 *      Author: huang630
 */

#include <algorithm>
#include <fcntl.h>
#include <ostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

//#include "Core/Debug.h"
//#include "Core/ProtoBuf.h"
#include "RaftLog.h"

namespace raftfs {
	namespace server {	// FIXME: put this under server first

#ifdef REF_FLUSH	// disabled
	/* FIXME: Need a function to flush log into storage */
	RaftLog::FlushStorage::FlushStorage(uint64_t lastIndex)
		: lastIndex(lastIndex)
		, completed(false) {
	}

	RaftLog::FlushStorage::~FlushStorage(){
		assert(completed);
	}
#endif

////////// Log //////////

RaftLog::RaftLog()
{
	memoryLog.clear();
}

RaftLog::~RaftLog()
{
	// TODO: check if we need to flush before closed.
}

// TODO: Finish these function bodies.
std::pair<uint64_t, uint64_t>  RaftLog::append(
					const std::vector<const Entry*>& entries)
{
	std::pair<uint64_t, uint64_t> rtn;
	rtn = std::make_pair(0,0);
	return rtn;
}

const Entry& RaftLog::getEntry(uint64_t index) const {
	return *(this->memoryLog[0]);
}

uint64_t RaftLog::getLogStartIndex() const {
	return memoryLog[0]->index;
}

uint64_t RaftLog::getLastLogIndex() const {
	const Entry * ptr = memoryLog.back();
	return ptr->index;


}

} // namespace raftfs::server
} // namespace raftfs
