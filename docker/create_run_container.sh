#!/usr/bin/env bash

cd ./compile/
sudo docker build -t fscompile .
cd ../../
sudo docker run -v $PWD:/src fscompile
cp build/raftfs docker/run/
cp build/fsclient docker/client/
cd docker/run/
sudo docker build -t fsrun .
cd ../client/
sudo docker build -t fsclient .