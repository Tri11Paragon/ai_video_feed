# Install script for directory: /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/lib/sample

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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/_testing" TYPE EXECUTABLE FILES "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/sample_lib_draw_rect")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect"
         OLD_RPATH "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_draw_rect:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib/3rd:/home/brett/Documents/code/milkv/video_feed/build/_deps_tdl/opencv-src/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/brett/Documents/code/milkv/video_feed/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_draw_rect")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/CMakeFiles/sample_lib_draw_rect.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/_testing" TYPE EXECUTABLE FILES "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/sample_lib_cvimd")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd"
         OLD_RPATH "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_md:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib/3rd:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/brett/Documents/code/milkv/video_feed/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_cvimd")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/CMakeFiles/sample_lib_cvimd.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/_testing" TYPE EXECUTABLE FILES "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/sample_lib_preprocess")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess"
         OLD_RPATH "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/cvi_preprocess:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tpu_musl_riscv64/cvitek_tpu_sdk/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib:/home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/cvi_mpi/lib/3rd:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/brett/Documents/code/milkv/video_feed/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/_testing/sample_lib_preprocess")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/CMakeFiles/sample_lib_preprocess.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/lib/sample/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
