#!/bin/sh
# Configure a Ninja build for helloworld on Windows with the MSYS2 CLANG64 toolchain.
# Needs the clang64 packages: clang, lld, cmake, ninja, qt6-base.
# usage: ./build-win.sh [Debug|Release]  -> build-win

MSYS2="${MSYS2:-/c/msys64}"
export PATH="$MSYS2/clang64/bin:$PATH"

cmake -G Ninja \
  -DCMAKE_BUILD_TYPE="${1:-Debug}" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_PREFIX_PATH="$MSYS2/clang64" \
  -S . -B build-win
