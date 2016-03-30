
namespace cpp raftfs.protocol

struct ReqVoteRequest {
1: required i32 term;
2: required i32 candidate_id;
3: optional i64 last_log_index;
4: optional i32 last_log_term;
}

struct ReqVoteResponse {
1: required i32 term;
2: required bool vote_granted;

}

service RaftService {
    ReqVoteResponse RequestVote(1:ReqVoteRequest vote);
}