#!/bin/sh

rm -rf build
rm -rf out

mkdir -p build/Debug
conan install . -of ./build/Debug --build missing -s build_type=Debug -s compiler=clang -s compiler.version=17

mkdir -p build/Release
conan install . -of ./build/Release --build missing -s build_type=Release -s compiler=clang -s compiler.version=17
