# Install script for directory: /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_draw_rect/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_md/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_preprocess/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_kit/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sample/cvi_md" TYPE FILE FILES
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/sample_lib_cvimd.cpp"
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/Makefile"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sample/cvi_preprocess" TYPE FILE FILES
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/sample_lib_preprocess.cpp"
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/Makefile"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/sample/cvi_draw_rect" TYPE FILE FILES
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/sample_lib_draw_rect.cpp"
    "/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample/Makefile"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
