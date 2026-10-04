# General Description
This project is a C++ executable for the Milk-v Duo S V1 (SG2000) meant to use hardware acceleration to run a small
neural network on the local TPU. The board has a low power RISC-V core so existing hardware acceleration must be used
(VDEC/VENC/VPSS + TPU). The full SDK tree lives inside this project:

# Locations
- `duo-buildroot-sdk-v2/tdl_sdk/`   - TDL SDK **source** (built from source via add_subdirectory; target `cvi_tdl-static`)
- `duo-buildroot-sdk-v2/cvi_mpi/`   - middleware headers (`include/`, `include/linux/`) + static libs (`lib/*.a`)
- `duo-buildroot-sdk-v2/cvi_rtsp/install/` - prebuilt cvi_rtsp (lib + headers)
- `duo-buildroot-sdk-v2/install/soc_sg2000_milkv_duos_musl_riscv64_sd/tmp-rootfs/` - board rootfs mirror (`mnt/system/lib` = the runtime generation on the board; `/mnt/tpu/tpu-sdk/lib` on the board is a DIFFERENT build, do not mix)
- `duo-buildroot-sdk-v2/host-tools/` (also `host-tools/` at project root) - riscv64 musl toolchain
- `duo-tdl-examples/` - Milk-V's reference TDL samples (`sample_vi_od` = person/vehicle detection reference)

# Build
`./build.sh` (toolchain file must be passed ABSOLUTE - tdl_sdk re-include()s it).
Library pairing is load-bearing: source-built cvi_tdl + cvi_mpi static libs + rootfs-mirror runtime .so's.
Vendor bugs fixed in-tree (git repo in tdl_sdk tracks them): printf spam in image/raw_image_classification
outputParser commented out; googletest add_subdirectory binary dir; opencv tegra_hal existence guard.
