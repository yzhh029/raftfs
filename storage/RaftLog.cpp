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

using namespace std;

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
	empty_ent.__set_index(-1);
	empty_ent.__set_term(-1);
}

RaftLog::~RaftLog()
{
	// TODO: check if we need to flush before closed.
}

// TODO: Finish these function bodies.
std::pair<uint64_t, uint64_t>  RaftLog::append(
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

const Entry& RaftLog::getEntry(uint64_t index) const {
	for(auto &&p: memoryLog) {
		if(p->index == index) {
			return *p;
		}
	}
	return empty_ent;
}

uint64_t RaftLog::getLogStartIndex() const {
	return memoryLog[0]->index;
}

uint64_t RaftLog::getLastLogIndex() const {
	const Entry * ptr = memoryLog.back();
	return ptr->index;


}

void RaftLog::removeEntryBefore(uint64_t firstIndex)
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

void RaftLog::removeEntryAfter(uint64_t firstIndex)
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


} // namespace raftfs::server
} // namespace raftfs
