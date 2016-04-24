
namespace cpp raftfs.protocol

struct FileInfo {
    1: required string path;
    2: required i64 size;
    3: required string creator;
    4: required i64 create_time;
}