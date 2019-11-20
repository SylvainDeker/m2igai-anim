#!/bin/sh

cd build
make -j 2
if [ $? -ne 0 ]
then
  exit -1
fi
cd ../Bundle-GNU/Debug/bin
./main-app -gpu
