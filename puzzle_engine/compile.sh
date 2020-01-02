#!/usr/bin/env bash
set -e

mkdir -p build-debug
pushd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
./frogs
./crossing
./family
popd

mkdir -p build-release
pushd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
./frogs
./crossing
./family
popd
