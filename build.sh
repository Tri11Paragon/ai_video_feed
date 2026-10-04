#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
mkdir -p build
# toolchain path MUST be absolute: tdl_sdk's CMakeLists re-include()s it
cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE="$(pwd)/cmake/riscv64-unknown-linux-musl.cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build ./build -j 32
