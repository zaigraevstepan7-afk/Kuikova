#!/usr/bin/env python3
"""Map class field offsets (and optional TypeInfo) from an old IL2CPP dump.cs
to a new dump.cs / script.json after a Standoff 2 update.

Usage:
  python3 map_offsets.py --old dump_old.cs --new dump_new.cs --input INPUT.txt
  python3 map_offsets.py --old dump_old.cs --new dump_new.cs --class PlayerController
  python3 map_offsets.py --script-old script_old.json --script-new script_new.json --typeinfo

INPUT.txt lines (any of):
  PlayerController.photon_player 0x158
  0x158  # with --class PlayerController
  PlayerManager_TypeInfo 0x92BD878
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


CLASS_RE = re.compile(
    r"public (?:abstract )?class (\w+)\b[^\n]*\n\{(.*?)\n\}",
    re.S,
)
FIELD_RE = re.compile(
    r"^\s*(?:private|public|protected|internal).*;\s*// 0x([0-9A-Fa-f]+)",
    re.M,
)
FIELD_NAMED_RE = re.compile(
    r"^\s*(?:private|public|protected|internal)\s+(.+?)\s+(\S+);\s*// 0x([0-9A-Fa-f]+)",
    re.M,
)


def parse_classes(dump_text: str) -> dict[str, list[tuple[int, str, str]]]:
    """classname -> list of (offset, type, field_name) in declaration order."""
    out: dict[str, list[tuple[int, str, str]]] = {}
    for m in CLASS_RE.finditer(dump_text):
        name, body = m.group(1), m.group(2)
        fields = []
        for fm in FIELD_NAMED_RE.finditer(body):
            typ, fname, off = fm.group(1).strip(), fm.group(2), int(fm.group(3), 16)
            fields.append((off, typ, fname))
        if fields:
            out[name] = fields
    return out


def map_class_fields(
    old_fields: list[tuple[int, str, str]],
    new_fields: list[tuple[int, str, str]],
) -> dict[int, int | None]:
    """Map old offset -> new offset using type signature sequence + relative index."""
    # Index old by offset (last wins if dup)
    old_by_off = {off: (i, typ, name) for i, (off, typ, name) in enumerate(old_fields)}
    # Build type-signature bags for fuzzy match
    mapping: dict[int, int | None] = {}
    for off, (idx, typ, name) in old_by_off.items():
        # Prefer same rough position: compare neighboring type chain
        best = None
        best_score = -1
        for j, (noff, ntyp, nname) in enumerate(new_fields):
            score = 0
            if ntyp == typ:
                score += 3
            # strip obfuscated names — type match is primary
            # proximity of index
            score += max(0, 5 - abs(j - idx))
            # check prev/next types
            if idx > 0 and j > 0 and old_fields[idx - 1][1] == new_fields[j - 1][1]:
                score += 2
            if idx + 1 < len(old_fields) and j + 1 < len(new_fields):
                if old_fields[idx + 1][1] == new_fields[j + 1][1]:
                    score += 2
            if score > best_score:
                best_score = score
                best = noff
        mapping[off] = best if best_score >= 3 else None
    return mapping


def load_typeinfo(script_path: Path) -> dict[str, int]:
    data = json.loads(script_path.read_text())
    out = {}
    for m in data.get("ScriptMetadata", []):
        name = m.get("Name") or ""
        if name.endswith("_TypeInfo"):
            out[name] = int(m["Address"])
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--old", type=Path, help="old dump.cs")
    ap.add_argument("--new", type=Path, help="new dump.cs")
    ap.add_argument("--input", type=Path, help="INPUT.txt with offsets to remap")
    ap.add_argument("--class", dest="classname", help="dump field table for one class")
    ap.add_argument("--script-old", type=Path)
    ap.add_argument("--script-new", type=Path)
    ap.add_argument("--typeinfo", action="store_true", help="map *_TypeInfo from script.json")
    ap.add_argument("--output", type=Path, default=Path("OUTPUT.txt"))
    args = ap.parse_args()

    if args.typeinfo:
        if not args.script_old or not args.script_new:
            print("--typeinfo needs --script-old and --script-new", file=sys.stderr)
            return 2
        old_ti, new_ti = load_typeinfo(args.script_old), load_typeinfo(args.script_new)
        lines = []
        for name, old_addr in sorted(old_ti.items()):
            new_addr = new_ti.get(name)
            if new_addr is None:
                # try short name match
                short = name.split(".")[-1]
                cands = [v for k, v in new_ti.items() if k.endswith(short)]
                new_addr = cands[0] if len(cands) == 1 else None
            if new_addr is None:
                lines.append(f"{name} 0x{old_addr:X} -> NOT FOUND")
            else:
                lines.append(f"{name} 0x{old_addr:X} -> 0x{new_addr:X} ({new_addr})")
        args.output.write_text("\n".join(lines) + "\n")
        print(f"Wrote {args.output} ({len(lines)} TypeInfo entries)")
        return 0

    if not args.old or not args.new:
        print("--old and --new dump.cs required (unless --typeinfo)", file=sys.stderr)
        return 2

    old_c = parse_classes(args.old.read_text(encoding="utf-8", errors="replace"))
    new_c = parse_classes(args.new.read_text(encoding="utf-8", errors="replace"))

    if args.classname:
        cls = args.classname
        if cls not in old_c or cls not in new_c:
            print(f"class {cls} missing in old or new dump", file=sys.stderr)
            return 1
        mp = map_class_fields(old_c[cls], new_c[cls])
        print(f"=== {cls} ===")
        print("OLD:")
        for off, typ, name in old_c[cls]:
            print(f"  0x{off:X}  {typ}  {name}")
        print("NEW:")
        for off, typ, name in new_c[cls]:
            print(f"  0x{off:X}  {typ}  {name}")
        print("MAP old->new:")
        for o, n in sorted(mp.items()):
            print(f"  0x{o:X} -> {('0x%X' % n) if n is not None else 'NOT FOUND'}")
        return 0

    if not args.input:
        print("provide --input or --class", file=sys.stderr)
        return 2

    # Precompute maps for all shared class names
    class_maps: dict[str, dict[int, int | None]] = {}
    for cls in set(old_c) & set(new_c):
        class_maps[cls] = map_class_fields(old_c[cls], new_c[cls])

    out_lines = []
    for raw in args.input.read_text().splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            out_lines.append(raw)
            continue
        # Class.field 0xOFF or Class 0xOFF
        m = re.match(
            r"^(?:(\w+)(?:\.(\S+))?\s+)?0x([0-9A-Fa-f]+)\s*$",
            line,
            re.I,
        )
        if not m:
            out_lines.append(raw + "  # UNPARSED")
            continue
        cls, field, off_s = m.group(1), m.group(2), m.group(3)
        off = int(off_s, 16)
        if cls and cls in class_maps:
            new_off = class_maps[cls].get(off)
            if new_off is None:
                out_lines.append(f"{line} -> NOT FOUND")
            else:
                label = f"{cls}.{field}" if field else cls
                out_lines.append(f"{label} 0x{off:X} -> 0x{new_off:X}")
        else:
            # search all classes
            hits = []
            for c, mp in class_maps.items():
                if off in mp and mp[off] is not None:
                    hits.append((c, mp[off]))
            if len(hits) == 1:
                c, n = hits[0]
                out_lines.append(f"{line} -> 0x{n:X}  [{c}]")
            elif not hits:
                out_lines.append(f"{line} -> NOT FOUND")
            else:
                opts = ", ".join(f"{c}:0x{n:X}" for c, n in hits[:5])
                out_lines.append(f"{line} -> AMBIGUOUS ({opts})")

    args.output.write_text("\n".join(out_lines) + "\n")
    print(f"Wrote {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
