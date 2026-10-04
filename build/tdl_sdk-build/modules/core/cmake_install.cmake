# Install script for directory: /home/brett/Documents/code/milkv/video_feed/duo-buildroot-sdk-v2/tdl_sdk/modules/core

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
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/liveness/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/ocr/ocr_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/yawn_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/smoke_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/eye_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/license_plate_recognition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/license_plate_keypoint/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_quality/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/fall_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/human_keypoints_detection/smooth_keypoints/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/mask_face_recognition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/isp_image_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/core/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/utils/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_attribute/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_attribute_cls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/mask_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/hand_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/hand_keypoint/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/hand_keypoint_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/osnet/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_detection/retina_face/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_detection/thermal_face_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_detection/face_mask_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolov8/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolov10/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolov3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolov5/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolov6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolo/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/yolox/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/ppyoloe/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/thermal_person_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/object_detection/mobiledetv2/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/deepsort/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/sound_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/segmentation/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/segmentation/topformer_seg/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/license_plate_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/face_landmarker/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/incar_object_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/ive/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/tamper_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/motion_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/human_keypoints_detection/yolov8_pose/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/human_keypoints_detection/simcc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/human_keypoints_detection/hrnet/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/image_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/raw_image_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/motion_segmentation/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/clip/clip_image/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/clip/clip_text/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/blip/blip_itm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/blip/blip_vqa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/blip/blip_cap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/open_object_detection/yolo_world_v2/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/instance_segmentation/yolov8_seg/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/lane_detection/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/lane_detection/polylanenet/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/super_resolution/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/occlusion_classification/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/ocr/ocr_recognition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/lane_detection/lstr/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/liveness/ir_liveness/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/depth_estimation/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/brett/Documents/code/milkv/video_feed/build/tdl_sdk-build/modules/core/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
