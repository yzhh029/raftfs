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

    string dir1("dir1"), dir2("dir2");

    shared_ptr<INode> dir1_node = make_shared<INodeDirectory>(dir1, owner, nullptr);
    cout << *dir1_node << " " << dir1_node->IsDir() << " "
                        << dir1_node->IsFile() << " " << dir1_node->IsRoot() << endl;

    shared_ptr<INodeDirectory> root = make_shared<INodeDirectory>(string(), "raftfs", nullptr);
    root->SetParent(root.get());
    cout << "root is root" << root->IsRoot() << endl;

    // test INodeDirectory add file
    auto dir1_ptr = static_pointer_cast<INodeDirectory>(dir1_node);
    cout << *dir1_ptr << endl;
    cout << dir1_ptr->CreateFile(f2) << endl;
    cout << *dir1_ptr << endl;

    cout << dir1_ptr->CreateDir(dir2) << endl;
    cout << *dir1_ptr << endl;

    auto f1_file = *static_pointer_cast<INodeFile>(f1_node);

    cout << dir1_ptr->AddFile(f1_file) << endl;
    cout << *dir1_ptr << endl;

    auto d2 = static_cast<INodeDirectory *>(dir1_ptr->GetChild(dir2));
    d2->CreateFile(f1);

    cout << *d2 << endl;
    cout << *dir1_ptr << endl;
}

int main(int argc, char** argv) {

    iNode_test();

    FSNamespace fs;
    char a[128] = "/a/b//c//d";
    char * pch;

    string owner("fake");

    //INodeFile t1("bad", owner, nullptr);
    //cout << t1.ValidName("/badname{") << endl;
    //cout << t1.ValidName("/goodfile") << endl;
/*
    cout << "mkdir /test1 : " << fs.MakeDir("/test1", owner, false) << endl;
    cout << "mkdir /test2 : " << fs.MakeDir("/test2", owner, false) << endl;
    //fs.list();
    cout << "mkdir test3: " << fs.MakeDir("test3", owner, false) << endl;
    cout << "mkdir /test1/test4" << fs.MakeDir("/test1/test4", owner, false) << endl;
    cout << "mkdir /null/test5" << fs.MakeDir("/null/test5", owner, false) << endl;
    */
    //fs.list();
    //cout << "Delete all childe" << fs.DeleteAllChild();

    return 0;

}
