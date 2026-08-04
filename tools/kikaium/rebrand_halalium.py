#!/usr/bin/env python3
"""
Build libkikaium.so from Halalium.

1) Brand: Lemming / Halalium / lemminghack → Kikaium (cheat feature names kept)
2) Menu chrome: rename tab labels so the UI does not look like a Halalium paste
"""
from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

# Exact byte replacements (new <= old; shorter → NUL-padded).
# Order: longer first.
BRAND: list[tuple[bytes, bytes]] = [
    (b"t.me/lemminghack, 0.39.2", b"Kikaium | private | 0392"),
    # If re-running on already-partially-branded SO:
    (b"t.me/kikaiumhack, 0.39.2", b"Kikaium | private | 0392"),
    (b"Halalium_Bypass", b"Kikaium_Bypass"),
    (b"Halalium_Hooks", b"Kikaium_Hooks"),
    (b"Kikaium_Bypass", b"Kikaium_Bypass"),  # no-op if already done
    (b"libhalalium.so", b"libkikaium.so"),
    (b"Lemming", b"Kikaium"),
]

# Null-bounded menu chrome only (avoid WildRage → WildNova etc.).
# Format: replace interior of \0OLD\0 with NEW (same length).
MENU_TABS: list[tuple[bytes, bytes]] = [
    (b"Rage", b"Nova"),
    (b"Visuals", b"Drawing"),
    (b"Misc", b"More"),
    (b"Settings", b"Controls"),
    (b"Skins", b"Items"),
    (b"Weapons", b"Loadout"),
    (b"World", b"Scene"),
    (b"Watermark", b"Hud Label"),
]

# Must remain intact (cheat / feature labels).
FORBIDDEN = [
    b"Silent Aim",
    b"Anti Aim",
    b"Enable Esp",
    b"Auto Wall",
    b"Through Walls",
    b"Wallshot",
    b"Enemy Chams",
    b"Local Chams",
    b"Inf Ammo",
    b"WildRage",  # skin name — do not corrupt
]


def replace_exact(data: bytearray, old: bytes, new: bytes) -> int:
    if len(new) > len(old):
        raise SystemExit(f"too long: {old!r} -> {new!r}")
    padded = new + b"\x00" * (len(old) - len(new))
    count = 0
    start = 0
    while True:
        i = data.find(old, start)
        if i < 0:
            break
        data[i : i + len(old)] = padded
        count += 1
        start = i + len(old)
    return count


def replace_nul_bounded(data: bytearray, old: bytes, new: bytes) -> int:
    """Replace only occurrences of \\0 + old + \\0 (keeps surrounding NULs)."""
    if len(new) != len(old):
        raise SystemExit(f"menu tab length mismatch: {old!r} -> {new!r}")
    needle = b"\x00" + old + b"\x00"
    count = 0
    start = 0
    while True:
        i = data.find(needle, start)
        if i < 0:
            break
        # write new between the NULs
        data[i + 1 : i + 1 + len(old)] = new
        count += 1
        start = i + 1 + len(old)
    return count


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--src", type=Path, default=Path("halalium/bin/libhalalium.so"))
    ap.add_argument("--dst", type=Path, default=Path("kikaium/bin/libkikaium.so"))
    args = ap.parse_args()

    if not args.src.is_file():
        print(f"missing source: {args.src}", file=sys.stderr)
        return 1

    raw = bytearray(args.src.read_bytes())

    print("brand:")
    for old, new in BRAND:
        if old == new:
            continue
        n = replace_exact(raw, old, new)
        print(f"  {old!r} -> {new!r} x{n}")

    print("menu tabs:")
    for old, new in MENU_TABS:
        n = replace_nul_bounded(raw, old, new)
        print(f"  {old!r} -> {new!r} x{n}")

    for s in FORBIDDEN:
        if s not in raw:
            # WildRage must exist; features should exist
            if s == b"WildRage" or s.startswith(b"Silent") or s.startswith(b"Anti") or s.startswith(b"Enable"):
                print(f"warning: missing expected string {s!r}", file=sys.stderr)

    leftovers = [t for t in (b"Halalium", b"Lemming", b"lemminghack", b"libhalalium", b"kikaiumhack") if t in raw]
    # Halalium substring check: Kikaium_Bypass doesn't contain Halalium
    if leftovers:
        print(f"error: leftovers {leftovers}", file=sys.stderr)
        return 2

    # Verify WildRage intact
    if b"WildRage" not in raw:
        print("error: WildRage skin string corrupted", file=sys.stderr)
        return 3

    args.dst.parent.mkdir(parents=True, exist_ok=True)
    args.dst.write_bytes(raw)
    also = Path("melodium/bin/libkikaium.so")
    also.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(args.dst, also)

    print(f"wrote {args.dst} ({len(raw)} bytes)")
    print(f"also {also}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
