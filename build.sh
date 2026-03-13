#!/bin/bash
# Fix for GCC temp file permission error on Windows:
# In a non-login bash (like Claude Code), MSYS2's /etc/profile doesn't run,
# so TMP stays as "/tmp". GCC (a native Windows binary) needs a Windows-style path.
# cygpath -m converts /tmp -> C:/Users/.../Temp (same as /etc/profile does normally).
WIN_TEMP=$(cygpath -m /tmp)
export TMP="$WIN_TEMP"
export TEMP="$WIN_TEMP"
export TMPDIR="$WIN_TEMP"

CMAKE="/c/Program Files/CMake/bin/cmake.exe"
mkdir -p build && cd build
"$CMAKE" ..
"$CMAKE" --build . --config Release