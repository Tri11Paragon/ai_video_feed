# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src")
  file(MAKE_DIRECTORY "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src")
endif()
file(MAKE_DIRECTORY
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-build"
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix"
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/tmp"
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp"
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/src"
  "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-subbuild/opencv-populate-prefix/src/opencv-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
