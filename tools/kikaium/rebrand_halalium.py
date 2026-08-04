#!/usr/bin/env python3
"""
Build libkikaium.so from Halalium.

Brand + menu chrome renames so the UI is not an obvious Halalium/Lemming paste.
Cheat feature labels (Silent Aim, Anti Aim, …) are left intact.
"""
from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

# Exact replacements (new <= old → NUL-padded). Longer first.
EXACT: list[tuple[bytes, bytes]] = [
    (b"from hehket: real thank u", b"from kikaium: built local"),
    (b"t.me/lemminghack, 0.39.2", b"Kikaium | private | 0392"),
    (b"t.me/kikaiumhack, 0.39.2", b"Kikaium | private | 0392"),
    (b"##rage_right_bottom", b"##nova_right_bottom"),
    (b"##misc_right_bottom", b"##more_right_bottom"),
    (b"##settings_watermark", b"##kikaium_wm_setting"),
    (b"##rage_right_top", b"##nova_right_top"),
    (b"##misc_right_top", b"##more_right_top"),
    (b"##settings_left", b"##controls_side"),
    (b"##rage_left", b"##nova_left"),
    (b"##misc_left", b"##more_left"),
    (b"##watermark", b"##nova_mark"),
    (b"##wm_click", b"##nova_clk"),
    (b"Halalium_Bypass", b"Kikaium_Bypass"),
    (b"Halalium_Hooks", b"Kikaium_Hooks"),
    (b"libhalalium.so", b"libkikaium.so"),
    (b"Accent Color", b"Theme Accent"),
    (b"Lemming", b"Kikaium"),
]

# Null-bounded tab / short labels (avoid WildRage etc.).
NUL_BOUNDED: list[tuple[bytes, bytes]] = [
    (b"Rage", b"Nova"),
    (b"Visuals", b"Drawing"),
    (b"Misc", b"More"),
    (b"Settings", b"Controls"),
    (b"Skins", b"Items"),
    (b"Weapons", b"Loadout"),
    (b"World", b"Scene"),
    (b"Watermark", b"Hud Label"),
    (b"Ping", b"Net "),
]

MUST_KEEP = [
    b"Silent Aim",
    b"Anti Aim",
    b"Enable Esp",
    b"WildRage",
]


def replace_exact(data: bytearray, old: bytes, new: bytes) -> int:
    if len(new) > len(old):
        raise SystemExit(f"too long: {old!r} -> {new!r}")
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
        raise SystemExit(f"length mismatch: {old!r} -> {new!r}")
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


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--src", type=Path, default=Path("halalium/bin/libhalalium.so"))
    ap.add_argument("--dst", type=Path, default=Path("kikaium/bin/libkikaium.so"))
    args = ap.parse_args()
    if not args.src.is_file():
        print(f"missing {args.src}", file=sys.stderr)
        return 1

    raw = bytearray(args.src.read_bytes())
    print("exact:")
    for old, new in EXACT:
        print(f"  {old!r} -> {new!r} x{replace_exact(raw, old, new)}")
    print("nul-bounded:")
    for old, new in NUL_BOUNDED:
        print(f"  {old!r} -> {new!r} x{replace_nul_bounded(raw, old, new)}")

    for s in MUST_KEEP:
        if s not in raw:
            print(f"error: missing {s!r}", file=sys.stderr)
            return 2

    bad = [t for t in (b"Halalium", b"Lemming", b"lemminghack", b"hehket", b"libhalalium", b"kikaiumhack") if t in raw]
    if bad:
        print(f"error: leftovers {bad}", file=sys.stderr)
        return 3

    args.dst.parent.mkdir(parents=True, exist_ok=True)
    args.dst.write_bytes(raw)
    also = Path("melodium/bin/libkikaium.so")
    also.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(args.dst, also)
    print(f"wrote {args.dst}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
