#!/bin/sh

dir="./test"
cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++
cmake -B build-release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++
cmake --build build
cmake --build build-release

for file in "$dir"/*.ir; do
   ./build/splc $file 
   #./build-release/splc $file
done
