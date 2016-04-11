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

#include "build/Protocol/Client.pb.h"
#include "build/Protocol/Raft.pb.h"
#include "Core/Debug.h"
#include "Core/ProtoBuf.h"
#include "RaftLog.h"

namespace LogCabin {
namespace Storage {

////////// Log::Sync //////////

Log::Sync::Sync(uint64_t lastIndex)
    : lastIndex(lastIndex)
    , completed(false) {
}

Log::Sync::~Sync()
{
    assert(completed);
}

////////// Log //////////

Log::Log()
    : metadata()
{
}

Log::~Log()
{
}


} // namespace LogCabin::Storage
} // namespace LogCabin
