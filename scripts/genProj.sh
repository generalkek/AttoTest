#!/bin/sh

config=Debug
if [ ! -z "$1" ];then
config=$1
fi

cmake -B ../build -S ../. -DCMAKE_CXX_COMPILER='gcc' -DCMAKE_BUILD_TYPE=$1