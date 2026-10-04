# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

if("/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz")
  message(FATAL_ERROR "File not found: /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz")
endif()

if("" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz'")

file("" "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/oss/oss_release_tarball/musl_riscv64/opencv_aisdk.tar.gz
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
