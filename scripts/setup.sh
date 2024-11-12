#!/bin/sh

if [ -z "$1" ];then
echo build Debug
./genProj.sh Debug
cmake --build ../build
else
echo build Release
./genProj.sh Release
cmake --build ../build
fi