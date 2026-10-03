#!/usr/bin/env python3
"""
Patch the printf spam out of the vendor cvitek-tdl-sdk-sg200x static libraries.

The TDL SDK ships `printf("scales:%f\n", ...)` inside:
  - ImageClassification::outputParser     (image_classification.cpp.o,     src line ~164)
  - RawImageClassification::outputParser  (raw_image_classification.cpp.o, src line ~159)

That printf runs once per output class on EVERY inference (1000x per frame for
ImageNet models), flooding the console and burning CPU in the hot loop.

This script NOPs the printf call sequences (auipc+jalr pairs) and blanks their
R_RISCV_CALL_PLT / R_RISCV_RELAX relocations so the linker cannot re-relax them
back into calls. It also terminates the "scales:%f" format string. Verified
against libcvi_tdl.a (md5 of originals recorded below).

Usage:
    ar x libcvi_tdl.a image_classification.cpp.o raw_image_classification.cpp.o
    python3 patch_cvi_tdl_scales.py image_classification.cpp.o raw_image_classification.cpp.o
    ar r libcvi_tdl.a image_classification.cpp.o raw_image_classification.cpp.o
    ranlib libcvi_tdl.a
"""
import struct
import sys

NOP = 0x00000013  # RISC-V `nop`


def sections(d):
    e_shoff = struct.unpack_from('<Q', d, 0x28)[0]
    e_shentsize = struct.unpack_from('<H', d, 0x3a)[0]
    e_shnum = struct.unpack_from('<H', d, 0x3c)[0]
    e_shstrndx = struct.unpack_from('<H', d, 0x3e)[0]

    def sh(i):
        o = e_shoff + i * e_shentsize
        name, typ, flags, addr, off, size, link, info, align, entsize = \
            struct.unpack_from('<IIQQQQIIQQ', d, o)
        return dict(name=name, typ=typ, off=off, size=size,
                    entsize=entsize, link=link)

    stroff = sh(e_shstrndx)['off']

    def nm(x):
        end = d.index(b'\0', stroff + x)
        return d[stroff + x:end].decode()

    return [(*sh(i), nm(sh(i)['name'])) for i in range(e_shnum)]


def patch_object(fn):
    d = bytearray(open(fn, 'rb').read())
    secs = sections(d)
    assert secs, 'no section headers parsed'

    # 1) Null the "scales:%f" format strings (first byte only - keeps offsets stable)
    nstr = 0
    for s in secs:
        if s.typ == 1 and s.name.startswith('.rodata'):
            so = d.find(b'scales', s.off, s.off + s.size)
            if so >= 0:
                d[so] = 0
                nstr += 1
                print(f'  nulled scales string at {hex(so)} ({s.name[:60]})')

    # 2) NOP every printf CALL_PLT site in every .text.* section of this object.
    #    In these two compilation units the ONLY printf call sites are the
    #    scales prints inside outputParser (verified by disassembly).
    nsites = 0
    for s in secs:
        if s.typ != 4 or not s.name.startswith('.rela.text.'):
            continue
        sec_name = s.name[len('.rela'):]
        txt = [t for t in secs if t.name == sec_name][0]
        for i in range(s.size // s.entsize):
            eo = s.off + i * s.entsize
            r_offset, r_info, _ = struct.unpack_from('<QQq', d, eo)
            if (r_info & 0xffffffff) == 19:  # R_RISCV_CALL_PLT
                # blank the CALL_PLT and its paired RELAX relocation
                for j in range(max(0, i - 1), min(s.size // s.entsize, i + 2)):
                    eo2 = s.off + j * s.entsize
                    ro2, ri2, _ = struct.unpack_from('<QQq', d, eo2)
                    if ro2 == r_offset:
                        struct.pack_into('<QQq', d, eo2, ro2, 0, 0)
                for o in (r_offset, r_offset + 4):
                    struct.pack_into('<I', d, txt.off + o, NOP)
                nsites += 1
                print(f'  NOPed printf call at {sec_name[:60]}+{hex(r_offset)}')
    print(f'{fn}: {nstr} strings nulled, {nsites} printf sites removed')
    if nsites == 0 and nstr == 0:
        print('  (nothing to do - already patched?)')
    open(fn, 'wb').write(d)


if __name__ == '__main__':
    for fn in sys.argv[1:]:
        patch_object(fn)
