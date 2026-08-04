#!/usr/bin/env python3
"""
Build libkikaium.so from Halalium: rename branding only.

Keeps cheat UI / feature names untouched (Silent Aim, Anti Aim, Enable Esp, …).
Shorter replacements are NUL-padded in-place so adjacent C-strings stay intact.
"""
from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

# (old, new) — new must be <= old length. Order: longest first to avoid partial hits.
REPLACEMENTS: list[tuple[bytes, bytes]] = [
    (b"t.me/lemminghack, 0.39.2", b"t.me/kikaiumhack, 0.39.2"),
    (b"Halalium_Bypass", b"Kikaium_Bypass"),
    (b"Halalium_Hooks", b"Kikaium_Hooks"),
    (b"libhalalium.so", b"libkikaium.so"),
    (b"Lemming", b"Kikaium"),
]

# Must NEVER be rewritten (cheat / feature labels).
FORBIDDEN_TOUCH = [
    b"Silent Aim",
    b"Anti Aim",
    b"Enable Esp",
    b"Auto Wall",
    b"Through Walls",
    b"Wallshot",
    b"Enemy Chams",
    b"Local Chams",
    b"Inf Ammo",
]


def apply(data: bytearray) -> list[str]:
    log: list[str] = []
    for old, new in REPLACEMENTS:
        if len(new) > len(old):
            raise SystemExit(f"replacement too long: {old!r} -> {new!r}")
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
        log.append(f"{old!r} -> {new!r} x{count}")
        if count == 0:
            print(f"warning: not found {old!r}", file=sys.stderr)
    return log


def verify_features_intact(data: bytes) -> None:
    for s in FORBIDDEN_TOUCH:
        if s not in data:
            print(f"warning: feature string missing after patch: {s!r}", file=sys.stderr)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--src",
        type=Path,
        default=Path("halalium/bin/libhalalium.so"),
        help="Halalium input SO",
    )
    ap.add_argument(
        "--dst",
        type=Path,
        default=Path("kikaium/bin/libkikaium.so"),
        help="Kikaium output SO",
    )
    args = ap.parse_args()

    if not args.src.is_file():
        print(f"missing source: {args.src}", file=sys.stderr)
        return 1

    raw = bytearray(args.src.read_bytes())
    before = bytes(raw)
    log = apply(raw)
    verify_features_intact(bytes(raw))

    # Sanity: no leftover Halalium / Lemming brand (case-sensitive product tags).
    leftovers = []
    for tag in (b"Halalium", b"Lemming", b"lemminghack", b"libhalalium"):
        if tag in raw:
            leftovers.append(tag)
    if leftovers:
        print(f"error: brand leftovers remain: {leftovers}", file=sys.stderr)
        return 2

    args.dst.parent.mkdir(parents=True, exist_ok=True)
    args.dst.write_bytes(raw)
    # Keep a sidecopy next to melodium downloads for convenience.
    also = Path("melodium/bin/libkikaium.so")
    also.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(args.dst, also)

    print(f"wrote {args.dst} ({len(raw)} bytes, delta vs src={len(raw) - len(before)})")
    for line in log:
        print(" ", line)
    print(f"also {also}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
