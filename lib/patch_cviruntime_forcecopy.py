#!/usr/bin/env python3
"""
Force CVI_NN device-memory input tensors to copy fresh data on EVERY forward pass.

The vendor cviruntime ships an "optimization" in cvi::runtime::Neuron::load():

    // needed to copy data using tdma.
    if (tensor.paddr != _paddr) {
        ...
        runtimeExecuteKernelFunction(_ctx, _streamCopyCmdbuf, tensor.paddr, _paddr);
    }

If the incoming frame's physical address equals the last one seen, the TDMA copy
is SKIPPED and the TPU re-runs on whatever was in its input buffer previously.
With VPSS preprocessing the resized 224x224 classification crop comes from a
small recycling VB pool, so the address repeats every frame while the CONTENT
changes - the model then returns identical predictions forever (frozen at
whatever the first frame was).

This script retargets that equality branch to the next instruction, making the
copy unconditional (the branch is dead either way), and blanks its
R_RISCV_BRANCH relocation so the linker cannot re-relax it.

Scope note: the input-copy guard is the ONLY stale-data path - Neuron::store
(outputs) always invalidates/copies unconditionally, so outputs stay fresh.

Usage (run from a scratch dir):
    ar x libcviruntime-static.a neuron.cpp.o
    python3 patch_cviruntime_forcecopy.py neuron.cpp.o
    cp libcviruntime-static.a libcviruntime-static.orig.a
    ar r libcviruntime-static.orig.a neuron.cpp.o && ranlib libcviruntime-static.orig.a

Verified against cvitek_tpu_sdk libcviruntime-static.a:
  Neuron::load @ .text+0x1ad2 (size 302), branch @ +0x1c = 0x00d60963
"""
import struct
import sys

FUNC_OFF = 0x1AD2   # Neuron::load offset inside .text of neuron.cpp.o
BRANCH = FUNC_OFF + 0x1C  # 'beq a2,a3' : paddr == _paddr -> skip TDMA copy
EXPECT = 0x00D60963


def sections(d):
    e_shoff = struct.unpack_from('<Q', d, 0x28)[0]
    e_shentsize = struct.unpack_from('<H', d, 0x3a)[0]
    e_shnum = struct.unpack_from('<H', d, 0x3c)[0]
    e_shstrndx = struct.unpack_from('<H', d, 0x3e)[0]

    def sh(i):
        o = e_shoff + i * e_shentsize
        name, typ, flags, addr, off, size, link, info, align, entsize = \
            struct.unpack_from('<IIQQQQIIQQ', d, o)
        return dict(name=name, typ=typ, off=off, size=size, entsize=entsize)

    stroff = sh(e_shstrndx)['off']

    def nm(x):
        end = d.index(b'\0', stroff + x)
        return d[stroff + x:end].decode()

    return [(*sh(i), nm(sh(i)['name'])) for i in range(e_shnum)]


def main(fn):
    d = bytearray(open(fn, 'rb').read())
    secs = sections(d)
    txt = [s for s in secs if s.typ == 1 and s.name == '.text'][0]
    cur = struct.unpack_from('<I', d, txt.off + BRANCH)[0]
    print(f'branch @ .text+{hex(BRANCH)} = {hex(cur)}')
    if cur == 0x00D60263:
        print('already patched (beq a2,a3,+4) - nothing to do')
        return
    assert cur == EXPECT, f'unexpected instruction {hex(cur)} - aborting'
    # keep rs1/rs2/funct3/opcode; zero imm; set imm[4:1]=0b0010 => offset +4
    new = (cur & ~0xF00) | 0x200
    struct.pack_into('<I', d, txt.off + BRANCH, new)
    imm = (((new >> 31) & 1) << 12) | (((new >> 7) & 1) << 11) | \
          (((new >> 25) & 0x3F) << 5) | (((new >> 8) & 0xF) << 1)
    assert imm == 4, f'encoding check failed: {imm}'
    print(f'patched to {hex(new)} (beq a2,a3,+4 = fall-through; copy always runs)')
    open(fn, 'wb').write(d)


if __name__ == '__main__':
    for f in sys.argv[1:]:
        main(f)
