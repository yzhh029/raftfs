//
// Created by yzhh on 4/22/16.
//

#include "../storage/LogManager.h"
#include <vector>
#include "../protocol/Raft_types.h"
#include "../protocol/Client_types.h"
#include <iostream>

using namespace raftfs::server;
using namespace raftfs::protocol;
using namespace std;

MetaOp::type randop() {
    return static_cast<MetaOp::type>(rand() % 5);
}

int main(int argc, char** argv) {

    LogManager log;

    vector<Entry> init_entries;

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


    return 0;

}
