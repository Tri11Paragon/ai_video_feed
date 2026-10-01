{ pkgs ? import <nixpkgs> {}}:
let
  rustupToolchain = "nightly";

  rustBuildTargetTripple = "riscv64gc-unknown-none-elf";
  rustBuildHostTripple = "x86_64-unknown-linux-gnu";

  milkv_chip = "CV181X";
  milkv_board = "riscv64";

  riscv-cross = import pkgs.path {
    crossSystem = {
      config = "riscv64-none-elf";
    };
  };

  riscv = riscv-cross.stdenv.cc;

in

pkgs.mkShell rec {
  buildInputs = with pkgs; [
    rustup
    riscv
    unixtools.xxd
    pkg-config
    udev
  ];

#  shellHook = ''
#
#    '';
}
