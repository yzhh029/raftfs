#!/usr/bin/env bash
thrift -gen cpp -out . Raft.thrift
rm RaftService_server.skeleton.cpp