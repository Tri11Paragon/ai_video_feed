cmake_minimum_required(VERSION 3.16)
project(video_feed)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

# tdl_sdk subbuild and its vendored Eigen are C++11-era: keep the GLOBAL standard
# at their generation; video_feed itself is switched to C++20 via target property.
set(CMAKE_CXX_STANDARD 11)

include_directories(include/)
include_directories(BLT/include)
include_directories(BLT/libraries/parallel-hashmap)
message("Sys: ${CMAKE_SYSROOT}")

# ------------------------------------------------------------------
# SDK layout (everything lives inside this project now)
# ------------------------------------------------------------------
set(SDK_ROOT       ${CMAKE_SOURCE_DIR}/duo-buildroot-sdk-v2)
set(MPI_ROOT       ${SDK_ROOT}/cvi_mpi)                                  # middleware source + static libs
set(TDL_SDK_ROOT   ${SDK_ROOT}/tdl_sdk)                                  # TDL source (built from source below)
set(RTSP_ROOT      ${SDK_ROOT}/cvi_rtsp)                                 # prebuilt rtsp (install/)
set(MPI_LIB        ${MPI_ROOT}/lib)
set(SDK_SYS_LIB    ${SDK_ROOT}/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tmp-rootfs/mnt/system/lib)
set(SDK_SYS_USR_LIB ${SDK_ROOT}/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tmp-rootfs/mnt/system/usr/lib)

# ------------------------------------------------------------------
# TDL SDK (source build) configuration.
# These must be set BEFORE add_subdirectory; tdl_sdk's cmake/*.cmake
# scripts consume them (same vars build_tdl_sdk.sh passes on the cmdline).
# ------------------------------------------------------------------
set(CVI_PLATFORM        CV181X)
set(MW_VER              v2)
set(USE_TPU_IVE         OFF)
set(CONFIG_DUAL_OS      OFF)   # tdl_sdk CMake compares this unquoted - must be defined
set(BUILD_OPTION        "")                                    # modules only (no samples/regression)
set(MIDDLEWARE_SDK_ROOT ${MPI_ROOT})
set(MLIR_SDK_ROOT       ${SDK_ROOT}/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk)
set(BUILD_DOWNLOAD_DIR  ${CMAKE_BINARY_DIR}/_deps_tdl)         # eigen/opencv/flatbuffers land here

if(NOT EXISTS ${MLIR_SDK_ROOT}/lib/libcviruntime.so)
    message(FATAL_ERROR "cvitek_tpu_sdk not found at ${MLIR_SDK_ROOT}")
endif()

add_subdirectory(${TDL_SDK_ROOT} ${CMAKE_BINARY_DIR}/tdl_sdk-build)

# The subbuild's static library target is 'cvi_tdl-static' (OUTPUT_NAME cvi_tdl).
set(TDL_TARGET cvi_tdl-static)

# ------------------------------------------------------------------
# OpenCV (fetched by the tdl_sdk subbuild into _deps_tdl)
# ------------------------------------------------------------------
set(OPENCV_DEP_DIR ${CMAKE_BINARY_DIR}/_deps_tdl/opencv-src)
set(OPENCV_3P ${OPENCV_DEP_DIR}/share/OpenCV/3rdparty/lib)

# ------------------------------------------------------------------
# Our executable
# ------------------------------------------------------------------
set(PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/yolo.cpp")

add_executable(video_feed ${PROJECT_BUILD_FILES}
    BLT/src/blt/parse/argparse_v2.cpp
    BLT/src/blt/std/string_algo.cpp
    BLT/src/blt/std/assert.cpp
    BLT/src/blt/logging/ansi.cpp
    BLT/src/blt/logging/fmt_tokenizer.cpp
    BLT/src/blt/logging/logging.cpp
    BLT/src/blt/logging/logging_config.cpp
    BLT/src/blt/logging/status.cpp
    BLT/src/blt/fs/stream_wrappers.cpp
    BLT/src/blt/fs/file_writers.cpp
    BLT/src/blt/fs/path_helper.cpp)

# $<TARGET_FILE:...> links by path, so the SDK libs need explicit build deps
add_dependencies(video_feed cvi_tdl-static cvi_kit-static)

target_compile_options(video_feed PRIVATE -Wall -Wno-comment
    -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d)
set_target_properties(video_feed PROPERTIES CXX_STANDARD 20)

# Board runtime search path: the rootfs /mnt/system libs are the SAME
# generation as this source-built TDL (that is the pairing the firmware
# itself ships), so the loader resolving libcviruntime.so etc. from
# /mnt/system/lib is correct and no LD_PRELOAD is needed.
target_link_options(video_feed PRIVATE
    -Wl,-rpath,/mnt/system/lib:/mnt/system/usr/lib:/usr/lib
    -Wl,-rpath-link,${SDK_SYS_LIB} -Wl,-rpath-link,${SDK_SYS_USR_LIB})

target_include_directories(video_feed PRIVATE
    ${TDL_SDK_ROOT}/include
    ${TDL_SDK_ROOT}/include/core
    ${TDL_SDK_ROOT}/include/cvi_tdl_app
    ${TDL_SDK_ROOT}/modules/lib/cvi_kit
    ${MPI_ROOT}/include
    ${MPI_ROOT}/include/linux
    ${RTSP_ROOT}/install/include)

target_link_directories(video_feed PUBLIC
    ${MPI_LIB}
    ${RTSP_ROOT}/install/lib
    ${SDK_SYS_LIB}
    ${SDK_SYS_USR_LIB}
    ${SDK_SYS_USR_LIB}/3rd)

target_link_libraries(video_feed PUBLIC
    # cvi_rtsp is the only dynamic SDK lib we need (matched generation ships on
    # the board's rootfs); everything static goes in one group so circular
    # archive deps resolve. Using TARGET_FILE paths avoids CMake re-ordering
    # the opencv archives behind the middleware ones.
    cvi_rtsp
    -Wl,--start-group
    $<TARGET_FILE:cvi_tdl-static>
    $<TARGET_FILE:cvi_kit-static>
    ${SDK_SYS_LIB}/libcviruntime.so
    ${SDK_SYS_LIB}/libcvikernel.so
    ${SDK_SYS_LIB}/libcvimath.so
    ${SDK_SYS_LIB}/libcnpy.so
    ${OPENCV_DEP_DIR}/lib/libopencv_core.a
    ${OPENCV_DEP_DIR}/lib/libopencv_imgproc.a
    ${OPENCV_DEP_DIR}/lib/libopencv_imgcodecs.a
    ${OPENCV_3P}/liblibwebp.a
    ${OPENCV_3P}/liblibjpeg.a
    ${OPENCV_3P}/liblibpng.a
    ${OPENCV_3P}/liblibtiff.a
    ${OPENCV_3P}/libIlmImf.a
    ${OPENCV_3P}/liblibjasper.a
    ${OPENCV_3P}/libzlib.a
    ${MPI_LIB}/libsys.a
    ${MPI_LIB}/libvdec.a
    ${MPI_LIB}/libvenc.a
    ${MPI_LIB}/libvpss.a
    ${MPI_LIB}/libvi.a
    ${MPI_LIB}/libvo.a
    ${MPI_LIB}/librgn.a
    ${MPI_LIB}/libgdc.a
    ${MPI_LIB}/libcvi_ive.a
    -Wl,--end-group
    pthread
    atomic
    m
    z)

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
