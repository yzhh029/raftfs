
namespace cpp raftfs.protocol

enum MetaOp {
    kMkdir,
    kRmdir,
    kCreate,
    kDelete,
    kNoop
}

struct Entry {
1: required i64 term;
2: required i64 index;
3: required MetaOp op;
4: required binary value;
}

struct ReqVoteRequest {
1: required i64 term;
2: required i32 candidate_id;
3: required i64 last_log_index;
4: required i64 last_log_term;
}

struct ReqVoteResponse {
1: required i64 term;
2: required bool vote_granted;
}

struct AppendEntriesRequest {
1: required i64 term;
2: required i32 leader_id;
3: required i64 prev_log_index;
4: required i64 prev_log_term;
5: required list<Entry> entries;
6: required i64 leader_commit_index;
}

struct AppendEntriesResponse {
1: required i64 term;
2: required bool success;
3: optional i64 last_log_index;
}

service RaftService {
    ReqVoteResponse RequestVote(1:ReqVoteRequest vote);
    AppendEntriesResponse AppendEntries(1:AppendEntriesRequest append);
}