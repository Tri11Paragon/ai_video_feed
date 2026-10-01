cmake_minimum_required(VERSION 3.5)
project(video_feed)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)

#add_subdirectory(BLT)

include_directories(include/)
include_directories(BLT/include)
include_directories(BLT/libraries/parallel-hashmap)
message("Sys: ${CMAKE_SYSROOT}")
#include_directories(${CMAKE_SYSROOT}/usr/include)

set(OPENCV_ENABLE_NONFREE ON)
set(OpenCV_DIR ${CMAKE_SYSROOT}/usr/lib/cmake/opencv4/)
find_package(OpenCV REQUIRED core videoio)

file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

#link_libraries(${CMAKE_SYSROOT}/usr/lib/)

add_executable(video_feed ${PROJECT_BUILD_FILES} BLT/src/blt/parse/argparse_v2.cpp BLT/src/blt/std/string_algo.cpp BLT/src/blt/std/assert.cpp BLT/src/blt/logging/ansi.cpp BLT/src/blt/logging/fmt_tokenizer.cpp BLT/src/blt/logging/logging.cpp BLT/src/blt/logging/logging_config.cpp BLT/src/blt/logging/status.cpp BLT/src/blt/fs/stream_wrappers.cpp BLT/src/blt/fs/file_writers.cpp BLT/src/blt/fs/path_helper.cpp)

target_compile_options(video_feed PRIVATE -Wall -Wno-comment -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d)
target_link_options(video_feed PRIVATE -Wall -Wno-comment -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -lavformat -lavdevice -lavcodec -lavutil -lswscale -lpthread)

target_include_directories(video_feed PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include/cvi_tdl
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include/cvi_tdl_app
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include/isp/cv181x
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/include/cvi_rtsp
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/tpu/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/duo-tdl-examples/include/system
    ${CMAKE_CURRENT_SOURCE_DIR}/duo-tdl-examples/include/tdl
)
target_link_libraries(video_feed PUBLIC
    ${OpenCV_LIBS}
    cvi_tdl
    cvi_rtsp
    cviruntime
    cvimath
    cvikernel
    cvi_ive
    cvi_draw_rect
    cvi_preprocess
    vdec
    venc
    vpss
    vi
    vo
    rgn
    gdc
    sys
    cvi_bin
    cvi_bin_isp
    misc
    sample
    pthread
    atomic
)
target_link_directories(video_feed PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/tpu/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/duo-tdl-examples/libs/system/musl_riscv64
    ${CMAKE_CURRENT_SOURCE_DIR}/duo-tdl-examples/libs/tdl/cv181x_riscv64
)

if (${ENABLE_ADDRSAN} MATCHES ON)
    target_compile_options(video_feed PRIVATE -fsanitize=address)
    target_link_options(video_feed PRIVATE -fsanitize=address)
endif ()

if (${ENABLE_UBSAN} MATCHES ON)
    target_compile_options(video_feed PRIVATE -fsanitize=undefined)
    target_link_options(video_feed PRIVATE -fsanitize=undefined)
endif ()

if (${ENABLE_TSAN} MATCHES ON)
    target_compile_options(video_feed PRIVATE -fsanitize=thread)
    target_link_options(video_feed PRIVATE -fsanitize=thread)
endif ()