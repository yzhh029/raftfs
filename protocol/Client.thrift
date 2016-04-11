
namespace cpp raftfs.protocol

struct GetLeaderResponse {
1: required i32 leader_id;
}

service ClientService {
    GetLeaderResponse GetLeader();
}