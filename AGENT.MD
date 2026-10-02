# General Description
This project is a C++ executable for the Milk-v DUO S V1 meant to use hardware acceleration to run a small neural network
on the local TPU. The board itself has a low power RISC-V core so if possible you must make use of existing hardware acceleration.
Code for the hardware acceleration is found in the `cvitek-tdl-sdk-sg200x` folder. The board makes use of the TPU to accelerate the neural network.

# Locations
Code for the hardware acceleration is found in the `cvitek-tdl-sdk-sg200x` folder.
    - This is explicitly meant for the correct Milk-v board.
Inside the `cvitek-tdl-sdk-sg200x` folder, you will find the necessary code to utilize the TPU for hardware acceleration.
    - Under `cvitek-tdl-sdk-sg200x/sample/3rd/middleware/v2/include` you will find additional hardware acceleration code.

# Requirements
- Make sure that you use the build scripts found alongside this document, build.sh and cleanbuild.sh.
- The build script is used to correctly cross-compile the video feed executable for the Milk-v DUO S V1.