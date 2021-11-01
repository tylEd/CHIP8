#!/bin/bash
set echo on

buildDir="./build/mac"
mkdir -p $buildDir

cppFiles=$(find src -type f -name "*.cpp")

exeName=$buildDir"/chip8"
compilerFlags="-g -std=c++11"
includeFlags=$(sdl2-config --cflags)
linkerFlags=$(sdl2-config --libs)
defines=""

echo "Building chip8 ..."
clang++ $cppFiles $compilerFlags -o $exeName $defines $includeFlags $linkerFlags

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
exit
fi

echo "Build Successful"
