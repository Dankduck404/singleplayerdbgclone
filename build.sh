#!/bin/bash
CMAKE="/c/Program Files/CMake/bin/cmake.exe"
mkdir -p build && cd build
"$CMAKE" ..
"$CMAKE" --build . --config Release