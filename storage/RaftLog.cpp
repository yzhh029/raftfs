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

//#include "Core/Debug.h"
//#include "Core/ProtoBuf.h"
#include "RaftLog.h"

namespace raftfs {
	namespace server {	// FIXME: put this under server first

RaftLog::FlushStorage::FlushStorage(uint64_t lastIndex)
    : lastIndex(lastIndex)
    , completed(false) {
}

RaftLog::FlushStorage::~FlushStorage(){
    assert(completed);
}

////////// Log //////////

RaftLog::RaftLog()
    //: metadata()
{
}

RaftLog::~RaftLog()
{
}


} // namespace LogCabin::Storage
} // namespace LogCabin
