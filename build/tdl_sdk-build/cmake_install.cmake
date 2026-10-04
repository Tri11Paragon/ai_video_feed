# Install script for directory: /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/brett/Documents/code/milkv/video_feed/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_core.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_core.so" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_core.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgproc.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_imgproc.so" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgproc.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgcodecs.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_imgcodecs.so" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgcodecs.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_core.so.3.2")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_core.so.3.2" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_core.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgproc.so.3.2")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_imgproc.so.3.2" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgproc.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgcodecs.so.3.2")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE PROGRAM RENAME "libopencv_imgcodecs.so.3.2" FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgcodecs.so.3.2.0")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_core.a;/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgproc.a;/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib/libopencv_imgcodecs.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/lib" TYPE FILE FILES
    "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_core.a"
    "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgproc.a"
    "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib/libopencv_imgcodecs.a"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/include/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/opencv/include" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/include/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib/libcvikernel.so;/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib/libcvimath.so;/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib/libcviruntime.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib" TYPE FILE FILES
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib/libcvikernel.so"
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib/libcvimath.so"
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib/libcviruntime.so"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib/libz.so.1")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib" TYPE PROGRAM RENAME "libz.so.1" FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib/libz.so.1.2.11")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib/libz.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/lib" TYPE PROGRAM RENAME "libz.so" FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib/libz.so.1.2.11")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/include/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/tpu/include" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/include/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/include/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/include" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/include/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/include/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/include" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/sample/common/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/lib/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/middleware/v2/lib" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib/")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/googletest-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sample/3rd/stb/include" TYPE DIRECTORY FILES "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/stb-src/" FILES_MATCHING REGEX "/[^/]*\\.h$" REGEX "/\\.git$" EXCLUDE REGEX "/\\.github$" EXCLUDE REGEX "/data$" EXCLUDE REGEX "/deprecated$" EXCLUDE REGEX "/docs$" EXCLUDE REGEX "/tests$" EXCLUDE REGEX "/tools$" EXCLUDE)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/rtsp/lib/libcvi_rtsp.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/rtsp/lib" TYPE FILE FILES "/cvi_rtsp/install/lib/libcvi_rtsp.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/rtsp/include/cvi_rtsp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/install/sample/3rd/rtsp/include" TYPE DIRECTORY FILES "/cvi_rtsp/install/include/cvi_rtsp")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
