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

#set(OPENCV_ENABLE_NONFREE ON)
#set(OpenCV_DIR ${CMAKE_SYSROOT}/usr/lib/cmake/opencv4/)
#find_package(OpenCV REQUIRED core videoio)

set(PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/yolo.cpp")

add_executable(video_feed ${PROJECT_BUILD_FILES} BLT/src/blt/parse/argparse_v2.cpp BLT/src/blt/std/string_algo.cpp BLT/src/blt/std/assert.cpp BLT/src/blt/logging/ansi.cpp BLT/src/blt/logging/fmt_tokenizer.cpp BLT/src/blt/logging/logging.cpp BLT/src/blt/logging/logging_config.cpp BLT/src/blt/logging/status.cpp BLT/src/blt/fs/stream_wrappers.cpp BLT/src/blt/fs/file_writers.cpp BLT/src/blt/fs/path_helper.cpp)

set(SDK_SYS_LIB /home/brett/Documents/code/milkv/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tmp-rootfs/mnt/system/lib)
set(SDK_SYS_USR_LIB /home/brett/Documents/code/milkv/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tmp-rootfs/mnt/system/usr/lib)

target_compile_options(video_feed PRIVATE -Wall -Wno-comment -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d)
target_link_options(video_feed PRIVATE -Wall -Wno-comment -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -lpthread -Wl,-rpath,/mnt/system/lib:/mnt/system/usr/lib:/usr/lib -Wl,-rpath-link,${SDK_SYS_LIB} -Wl,-rpath-link,${SDK_SYS_USR_LIB})

target_include_directories(video_feed PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/opencv/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include/cvi_tdl
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/include/cvi_tdl_app
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include/isp/cv181x
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/include/cvi_rtsp
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/utils
)
target_link_libraries(video_feed PUBLIC
    -Wl,--start-group
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib/libcvi_tdl.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib/libcvi_draw_rect.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib/libcvi_preprocess.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib/libcvi_md.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libvpu.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libsys.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libvdec.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libvenc.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libcvi_ive.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libcvi_bin.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libcvi_bin_isp.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libisp.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libisp_algo.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libae.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libawb.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libaf.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libsample.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libsns_full.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/libmisc.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/3rd/libini.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/3rd/libcli.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/3rd/libjson-c.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/opencv/lib/libopencv_imgcodecs.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/opencv/lib/libopencv_imgproc.a
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/opencv/lib/libopencv_core.a
    -Wl,--end-group
    cvi_rtsp
    cviruntime
    cvimath
    cvikernel
    cnpy
    pthread
    atomic
    m
    z
)
target_link_directories(video_feed PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/opencv/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/tpu/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/lib/3rd
    ${CMAKE_CURRENT_SOURCE_DIR}/cvitek-tdl-sdk-sg200x/sample/3rd/rtsp/lib
    ${SDK_SYS_LIB}
    ${SDK_SYS_USR_LIB}
    ${SDK_SYS_USR_LIB}/3rd
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