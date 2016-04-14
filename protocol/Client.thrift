
namespace cpp raftfs.protocol

enum Status {
    kOK,
    kNoLeader,
    kNotFound,
    kExist
}

struct GetLeaderResponse {
1: required Status status;
2: required i32 leader_id;
}

struct MkdirRequest {
1: required string path;
}

struct MkdirResponse {
1: required Status status;
}

service ClientService {
    GetLeaderResponse GetLeader();
    MkdirResponse Mkdir(1:MkdirRequest new_dir)
}