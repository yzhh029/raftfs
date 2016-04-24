#!/usr/bin/env bash
thrift -gen cpp -out . Filesystem.thrift
thrift -gen cpp -out . Raft.thrift
thrift -gen cpp -out . Client.thrift
rm *.skeleton.cpp