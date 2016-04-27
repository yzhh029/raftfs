#!/usr/bin/env bash

cd ./compile/
sudo docker build -t fscompile .
cd ../../
sudo docker run -v $PWD:/src fscompile