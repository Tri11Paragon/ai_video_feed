set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(CMAKE_C_COMPILER "${CMAKE_SOURCE_DIR}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "${CMAKE_SOURCE_DIR}/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-g++")

set(CMAKE_SYSROOT "/home/brett/Documents/code/milkv/duo-buildroot-sdk-v2/buildroot/output/milkv-duos-musl-riscv64-sd/host/riscv64-buildroot-linux-musl/sysroot/")
set(BUILDROOT_SDK "/home/brett/Documents/code/milkv/duo-buildroot-sdk-v2/")

set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS_INIT   "-Os")
set(CMAKE_CXX_FLAGS_INIT "-Os")

set(CMAKE_C_COMPILER_TARGET riscv64-unknown-linux-musl)
set(CMAKE_CXX_COMPILER_TARGET riscv64-unknown-linux-musl)
