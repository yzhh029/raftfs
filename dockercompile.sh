#!/usr/bin/env bash
cd ./protocol/ && ./compileThrift.sh
cd ../ 
$CMAKE .
make all

