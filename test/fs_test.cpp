//
// Created by huang630 on 4/23/16.
//

#include "../filesystem/FSNamespace.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace raftfs::filesystem;
using namespace raftfs::protocol;
using namespace std;

int main(int argc, char** argv) {

    FSNamespace fs;
    char a[128] = "/a/b//c//d";
    char * pch;

    istringstream f(a);
    string s;

    while (getline(f, s, '/')) {
    	if(s.compare("") != 0)
    	cout << s << " rest: " << f.eof() << endl;
    }

    string owner("fake");

    cout << "mkdir /test1 : " << fs.MakeDir("/test1", owner, false) << endl;
    cout << "mkdir /test2 : " << fs.MakeDir("test2", owner, false) << endl;
    //fs.list();
    cout << "mkdir test3: " << fs.MakeDir("test3", owner, false) << endl;
    cout << "mkdir /test1/test4" << fs.MakeDir("/test1/test4", owner, false) << endl;
    cout << "mkdir /null/test5" << fs.MakeDir("/null/test5", owner, false) << endl;
    //fs.list();


#if(0)

    // test single append
    cout << "test single append" << endl;
    for (int i = 1; i <= 10; ++i) {
        Entry* e = new Entry();
        e->term = 1;
        e->index = i;
        e->op = randop();
        log.Append(e);
        cout << log;
    }


    cout << "test batch append" << endl;
    vector<Entry> batch;
    for (int i = 11; i <= 15; ++i) {
        Entry e;
        e.term = 1;
        e.index = i;
        e.op = randop();
        batch.push_back(e);
    }
    cout << log.Append(&batch);
    cout << log << endl;

    cout << "test conflict append" << endl;
    vector<Entry> conflict;
    Entry same;
    same.term = 1;
    same.index = 9;
    same.op = randop();
    conflict.push_back(same);
    cout << same.index << ":" << same.op << " ";
    for (int i = 12; i <= 20; ++i) {
        Entry e;
        e.term = 2;
        e.index = i -2;
        e.op = randop();
        cout << e.index << ":" << e.op << " ";
        conflict.push_back(e);
    }
    cout << endl;
    cout << log.Append(&conflict);
    cout << log << endl;

#endif

    return 0;

}
