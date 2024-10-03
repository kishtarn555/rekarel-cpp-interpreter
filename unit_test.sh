#!/bin/bash

set -e

echo "== cmake -S . -B build =="
cmake -S . -B build

echo "== cmake --build build =="
cmake --build build
cd build
echo "== ctest =="
ctest --output-on-failure
cd ..