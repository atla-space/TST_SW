#!/bin/sh

rm -rf build
rm -rf out

mkdir -p build/Debug
cd ./build/Debug
conan install ../.. --build missing -s build_type=Debug -g cmake_find_package_multi -s compiler=clang -s compiler.version=16
cd ../..

mkdir -p build/Release
cd ./build/Release
conan install ../.. --build missing -s build_type=Release -g cmake_find_package_multi -s compiler=clang -s compiler.version=16
cd ../..
