#!/usr/bin/env bash
cd ./protocol/ && ./compileThrift.sh
cd ../ && mkdir build
cd build && $CMAKE ..
echo $PWD
ls -al
make all
make clean
