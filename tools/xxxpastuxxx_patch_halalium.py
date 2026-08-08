#!/usr/bin/env python3
"""
Patch ORIGINAL libhalalium.so → libxxxpastuxxx.so

This is binary rebrand of the uploaded Halalium SO (aka "libhalalium (1).so"),
NOT a rebuild of Kikaium sources.

- Brand: xxxpastuxxx
- TG:    t.me/xxxstuxxx
- Menu ImGui ids remapped (new chrome names)
"""
from __future__ import annotations

import argparse
import re
import shutil
import struct
from pathlib import Path

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from elftools.elf.elffile import ELFFile

# Exact replacements (new <= old → NUL-padded). Longer first.
EXACT: list[tuple[bytes, bytes]] = [
    # Credits / watermark lines
    (b"from hehket: real thank u", b"\x00" + b"\x00" * 24),  # wipe credit (empty)
    (b"t.me/lemminghack, 0.39.2", b"t.me/xxxstuxxx"),  # watermark TG only
    (b"##rage_right_bottom", b"##stx_right_bottom"),
    (b"##misc_right_bottom", b"##utl_right_bottom"),
    (b"##vis_right_bottom", b"##sgt_right_bottom"),
    (b"##settings_watermark", b"##stux_wm_setting"),
    (b"##rage_right_top", b"##stx_right_top"),
    (b"##misc_right_top", b"##utl_right_top"),
    (b"##vis_right_top", b"##sgt_right_top"),
    (b"##weapons_list", b"##loadout_list"),
    (b"##settings_left", b"##sys_left_pane"),
    (b"##skins_panel", b"##skin_panel0"),
    (b"##skins_list", b"##skin_list0"),
    (b"##rage_left", b"##stx_left0"),
    (b"##misc_left", b"##utl_left0"),
    (b"##vis_left", b"##sgt_left"),
    (b"##watermark", b"##stux_mark"),
    (b"##wm_click", b"##stux_clk"),
    (b"##tab_bar", b"##stx_bar"),
    (b"##tabbtn", b"##stxbtn"),
    (b"Halalium_Bypass", b"xxxpastuxxx_BP"),
    (b"Halalium_Hooks", b"xxxpastuxxx_Hk"),
    (b"libhalalium.so", b"libxxxpastu.so"),
]

# Null-bounded tab labels — more distinct from Halalium names
NUL_BOUNDED: list[tuple[bytes, bytes]] = [
    (b"Rage", b"Aim "),
    (b"Visuals", b"ESP    "),
    (b"Misc", b"More"),
    (b"Settings", b"Config  "),
    (b"Skins", b"Paint"),
    (b"Watermark", b"Hud Label"),
]

MUST_GONE = [
    b"Lemming",
    b"lemminghack",
    b"Halalium",
    b"libhalalium",
    b"hehket",
    b"real thank",
]

# "Lemming" (7) cannot hold "xxxpastuxxx" (11) — plant + retarget ADRP/ADD.
PLANT_BRAND = b"xxxpastuxxx\x00"
OLD_BRAND = b"Lemming\x00"
MUST_KEEP = [b"Silent Aim", b"Enable Esp", b"Anti Aim"]


def replace_exact(data: bytearray, old: bytes, new: bytes) -> int:
    if len(new) > len(old):
        raise SystemExit(f"too long: {old!r} -> {new!r} ({len(new)}>{len(old)})")
    padded = new + b"\x00" * (len(old) - len(new))
    n = 0
    start = 0
    while True:
        i = data.find(old, start)
        if i < 0:
            break
        data[i : i + len(old)] = padded
        n += 1
        start = i + len(old)
    return n


def replace_nul_bounded(data: bytearray, old: bytes, new: bytes) -> int:
    if len(new) != len(old):
        raise SystemExit(f"len mismatch: {old!r}->{new!r}")
    needle = b"\x00" + old + b"\x00"
    n = 0
    start = 0
    while True:
        i = data.find(needle, start)
        if i < 0:
            break
        data[i + 1 : i + 1 + len(old)] = new
        n += 1
        start = i + 1 + len(old)
    return n


def find_zero_gap(data: bytes, need: int, ro_off: int, ro_size: int) -> int:
    """Return file offset of zero run length >= need inside .rodata."""
    end = ro_off + ro_size
    j = ro_off
    best = -1
    while j < end:
        if data[j] == 0:
            k = j
            while k < end and data[k] == 0:
                k += 1
            if k - j >= need + 8:  # leave a little padding
                # prefer mid-sized gaps not at very start
                return j + 4
            j = k
        else:
            j += 1
    raise SystemExit("no zero gap for brand plant")


def patch_adrp_add_target(data: bytearray, text_va: int, text_off: int, text_size: int,
                          old_va: int, new_va: int) -> int:
    """Rewrite ADRP+ADD pairs that materialize old_va to materialize new_va."""
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    chunk = bytes(data[text_off : text_off + text_size])
    pending: dict[str, tuple[int, int]] = {}  # reg -> (page, adrp_file_off)
    patched = 0
    for insn in md.disasm(chunk, text_va):
        fo = text_off + (insn.address - text_va)
        if insn.mnemonic == "adrp":
            m = re.match(r"(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)", insn.op_str)
            if m:
                pending[m.group(1)] = (int(m.group(2), 0), fo)
        elif insn.mnemonic == "add":
            m = re.match(r"(x\d+),\s*(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)$", insn.op_str)
            if not m or m.group(1) != m.group(2) or m.group(1) not in pending:
                continue
            reg = m.group(1)
            page, adrp_fo = pending[reg]
            cur = page + int(m.group(3), 0)
            if cur != old_va:
                continue
            # Build new ADRP+ADD for new_va at adrp PC
            pc = insn.address - 4  # adrp address
            new_page = new_va & ~0xFFF
            add_imm = new_va & 0xFFF
            # ADRP encoding
            rd = int(reg[1:])
            imm = (new_page - (pc & ~0xFFF)) >> 12
            imm &= (1 << 21) - 1
            immlo = imm & 3
            immhi = (imm >> 2) & 0x1FFFFF
            adrp_word = 0x90000000 | (immlo << 29) | (immhi << 5) | rd
            # ADD Xd, Xn, #imm12 (64-bit)
            add_word = 0x91000000 | (add_imm << 10) | (rd << 5) | rd
            struct.pack_into("<I", data, adrp_fo, adrp_word)
            struct.pack_into("<I", data, fo, add_word)
            patched += 1
            print(f"  retarget ADRP+ADD @ {pc:#x}: {old_va:#x} -> {new_va:#x}")
    return patched


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", type=Path, default=Path("/tmp/libhalalium.so"))
    ap.add_argument("--dst", type=Path, default=Path("halalium/bin/libxxxpastuxxx.so"))
    args = ap.parse_args()

    raw = bytearray(args.src.read_bytes())
    with args.src.open("rb") as f:
        elf = ELFFile(f)
        ro = elf.get_section_by_name(".rodata")
        text = elf.get_section_by_name(".text")
        ro_off, ro_va, ro_size = ro["sh_offset"], ro["sh_addr"], ro["sh_size"]
        text_off, text_va, text_size = text["sh_offset"], text["sh_addr"], text["sh_size"]

    # Identity map for this ELF (file off == VA for loadable)
    old_brand_va = raw.find(OLD_BRAND)
    if old_brand_va < 0:
        raise SystemExit("Lemming not found")
    print(f"old Lemming @ {old_brand_va:#x}")

    plant_off = find_zero_gap(raw, len(PLANT_BRAND), ro_off, ro_size)
    plant_va = plant_off  # identity
    raw[plant_off : plant_off + len(PLANT_BRAND)] = PLANT_BRAND
    print(f"planted xxxpastuxxx @ {plant_va:#x}")

    n_ret = patch_adrp_add_target(raw, text_va, text_off, text_size, old_brand_va, plant_va)
    if n_ret < 1:
        raise SystemExit("failed to retarget Lemming xrefs")
    # Wipe old Lemming so leftovers check passes
    raw[old_brand_va : old_brand_va + 7] = b"xxxxxxx"

    print("exact:")
    for old, new in EXACT:
        c = replace_exact(raw, old, new)
        print(f"  {old!r} -> {new!r} x{c}")

    print("nul-bounded:")
    for old, new in NUL_BOUNDED:
        c = replace_nul_bounded(raw, old, new)
        print(f"  {old!r} -> {new!r} x{c}")

    for s in MUST_KEEP:
        if s not in raw:
            print(f"error: missing feature {s!r}")
            return 2

    # leftover brand check (allow wiped xxxxxxx)
    leftovers = []
    for t in MUST_GONE:
        if t in raw:
            leftovers.append(t)
    if leftovers:
        print(f"error: leftovers {leftovers}")
        return 3

    if b"xxxpastuxxx" not in raw or b"t.me/xxxstuxxx" not in raw:
        print("error: new brand missing")
        return 4

    args.dst.parent.mkdir(parents=True, exist_ok=True)
    args.dst.write_bytes(raw)
    # also convenience copies
    for p in (Path("kikaium/bin/libxxxpastuxxx-from-halalium.so"),
              Path("melodium/bin/libxxxpastuxxx-from-halalium.so")):
        p.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(args.dst, p)
    print(f"wrote {args.dst} ({len(raw)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
