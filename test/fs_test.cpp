//
// Created by huang630 on 4/23/16.
//

#include "../filesystem/FSNamespace.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>
#include "../filesystem/INode.h"
#include "../protocol/Filesystem_types.h"
#include "../utils/time_utils.h"

using namespace raftfs::filesystem;
using namespace raftfs::protocol;
using namespace std;

void iNode_test() {
    string owner("fakeowner");
    string f1("file1");
    string f2("file2");
    shared_ptr<INode> f1_node = make_shared<INodeFile>(f1, nullptr);
    cout << *f1_node << " " << f1_node->IsDir() << " "
                            << f1_node->IsFile() << " " << f1_node->IsRoot() <<endl;

    string dir1("dir1"), dir2("dir2"), dir3("dir3");

    shared_ptr<INode> dir1_node = make_shared<INodeDirectory>(dir1, owner, nullptr);
    cout << *dir1_node << " " << dir1_node->IsDir() << " "
                        << dir1_node->IsFile() << " " << dir1_node->IsRoot() << endl;

    // test root
    shared_ptr<INodeDirectory> root = make_shared<INodeDirectory>(string(), "raftfs", nullptr);
    root->SetParent(root.get());
    cout << "root is root" << root->IsRoot() << endl;

    // test INodeDirectory add file
    auto dir1_ptr = static_pointer_cast<INodeDirectory>(dir1_node);

    cout << "add file2 " << dir1_ptr->CreateFile(f2) << endl;
    cout << *dir1_ptr << endl;

    // add directory
    cout << "add dir2 " << dir1_ptr->CreateDir(dir2) << endl;
    cout << *dir1_ptr << endl;

    // add same file again, should fail
    cout << "add file2 again " << dir1_ptr->CreateFile(f2) << endl;
    cout << *dir1_ptr << endl;


    FileInfo f2_info;

    static_cast<INodeFile *>(dir1_ptr->GetChild(f2))->ToFileInfo(f2_info);
    cout << "file2 info " << f2_info.path << " " << f2_info.creator << " " << f2_info.create_time << endl;
    cout << chrono::duration_cast<chrono::seconds>(chrono::system_clock::now()
                                                           .time_since_epoch()).count() << endl;

    cout << "get file2 " << dir1_ptr->GetChild(f2)->GetName() << endl;
    cout << "get file1 " << dir1_ptr->GetChild(f1) << endl;
    cout << "get dir2 " << dir1_ptr->GetChild(dir2)->GetName() << endl;

    auto dirs = dir1_ptr->ListDirName();

    for (auto& n: dirs) {
        cout << n << endl;
    }

    cout << " add dir3 to dir2 " << static_cast<INodeDirectory *>(dir1_ptr->GetChild(dir2))->CreateDir(dir3) << endl;
    cout << " add file1 to dir2 " << static_cast<INodeDirectory *>(dir1_ptr->GetChild(dir2))->CreateFile(f1) << endl;
    cout << *dir1_ptr << endl;

    // remove dir2
    cout << "delete dir2 " << dir1_ptr->DeleteChild(dir2, true) << endl;
    cout << *dir1_ptr << endl;
}

void fs_test() {
    FSNamespace fs;

    string owner("fakeowner");

    cout << " try mkdir /aaa should 1 " << fs.MakeDir("/aaa", owner, false) << endl;
    cout << " try mkdir /abc/def should 0 " << fs.MakeDir("/abc/def", owner, false) << endl;
    cout << " try mkdir /abc/def should 1 " << fs.MakeDir("/abc/def", owner, true) << endl;
    fs.Print();

    cout << " try mkdir /abc/def/ghk should 1 " << fs.MakeDir("/abc/def/ghk", owner, false) << endl;
    fs.Print();

    cout << " try rmdir /abc/def should 0 " << fs.DeleteDir("/abc/def", owner, false) << endl;
    cout << " try rmdir /abc/def/ should 1 " << fs.DeleteDir("/abc/def", owner, true) << endl;
    fs.Print();

    cout << " try create file /abc/abc shoud 1 " << fs.CreateFile("/abc/abc", owner) << endl;
    fs.Print();
    cout << " try create file /flaskdfj/asldkfj should 0 " << fs.CreateFile("/flaskdfj/asldkfj", owner) << endl;
    fs.Print();
}

int main(int argc, char** argv) {

    //iNode_test();

    fs_test();

    return 0;

}
