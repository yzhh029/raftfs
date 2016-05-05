
include "Filesystem.thrift"

namespace cpp raftfs.protocol

enum Status {
    kOK,
    kNoLeader,
    kNotLeader,
    kNotFound,
    kTimeout,
    kExist,
    kCommError,
    kPathError
}

struct GetLeaderResponse {
1: required Status status;
2: required i32 leader_id;
}

struct MkdirRequest {
1: required string path;
2: optional bool make_parents;
}

struct MkdirResponse {
1: required Status status;
2: optional i32 leader_id;
}

struct RmdirRequest {
1: required string dir;
2: optional bool recursive;
}

struct RmdirResponse {
1: required Status status;
2: optional i32 leader_id;
}

struct CreateFileRequest {
1: required string file_name;
2: optional i32 mode;
}

struct CreateFileResponse {
1: required Status status;
2: optional i32 leader_id;
}

struct DeleteRequest {
1: required string path;
2: optional bool recursive;
}

struct DeleteResponse {
1: required Status status;
2: optional i32 leader_id;
}

struct FileInfoRequest {
1: required string file;
}

struct FileInfoResponse {
1: required Status status;
2: required Filesystem.FileInfo info;
}

struct ListDirRequest {
1: required string dir;
}

struct ListDirResponse {
1: required Status status;
2: optional list<string> dir_list;
}

struct TestCaseRequest {
1: required i32 test_type;
}

struct TestCaseResponse {
1: required Status status;
}

service ClientService {
    GetLeaderResponse GetLeader();
    MkdirResponse Mkdir(1: MkdirRequest new_dir);
    RmdirResponse Rmdir(1: RmdirRequest dir);
    CreateFileResponse CreateFile(1: CreateFileRequest new_file);
    DeleteResponse DeleteFile(1: DeleteRequest path);
    FileInfoResponse GetFileInfo(1: FileInfoRequest file);
    ListDirResponse ListDir(1: ListDirRequest dir);
    TestCaseResponse InjectTestCase(1: TestCaseRequest req);
}