#!/usr/bin/env python3
"""Halalium 2-pass decompile: Pass A = Capstone disasm, Pass B = fields/RVAs/ADRP+ADD strings.

Usage:
  python3 tools/halalium_emu/decompile_funcs.py
  # Two conceptual commands per function — both run inside this tool:
  #   (1) cs.disasm  (2) field/RVA/string recovery
"""
from __future__ import annotations

import json
import re
import struct
from pathlib import Path

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from capstone.arm64 import ARM64_OP_IMM

ROOT = Path(__file__).resolve().parents[2]
SO_PATH = ROOT / "halalium/bin/libhalalium.so"
OUT = ROOT / "kikaium/docs/decompile"

ALLOW_UI = {
    "Enable Esp", "Box", "Box Type", "Box Color", "Health Bar", "Bone", "Distance",
    "Distance Color", "Fov Check", "Fov", "Fov Color", "scope fov", "Chams",
    "Enemy Chams", "Enemy Color", "Local Chams", "Local Color", "Through Walls",
    "Third Person", "Inf Ammo", "Fire Rate", "Wallshot", "Skin Changer", "Silent Aim",
    "Auto Fire", "Auto Wall", "No spread", "Anti Aim", "Anti Aim Pitch",
    "Enable Anti Aim first", "Spin", "Spin Speed", "Reverse Spin", "World",
    "World Color", "Solid World Color", "Apply World Color", "Watermark",
    "Corner Size", "Accent Color", "##vis_left", "##vis_right_top", "##vis_right_bottom",
    "##rage_left", "##rage_right_top", "##rage_right_bottom", "##misc_left",
    "##misc_right_top", "##settings_left", "##settings_watermark", "##skins_panel",
    "##wm_click", "Halalium_Hooks", "Halalium_Bypass", "InstantiateViaServer",
    "eglSwapBuffers", "libEGL.so", "libunity.so", "libinput.so",
    "got call from getrr.", "bypas hok result %d", "SkinChanger",
    "Skin Changer: Swapped to weapon %d (skin %d)",
}

FIELD_HINTS = {
    0x28: "players_list / camera holder",
    0x30: "Photon.isLocal (ldrb) / nested",
    0x79: "Player.team (byte)",
    0x88: "Player.weaponry",
    0x98: "Player.movement",
    0xA0: "Weaponry.weapon OR arms",
    0xA8: "Player.hit / weapon params",
    0xD8: "Player.visible (strb)",
    0xE8: "Player.main_camera",
    0x160: "Player.photon_player",
}

FUNCS = [
    ("egl_install", 0x1D84CC, 0x200),
    ("egl_callback", 0x1D76F0, 0x320),
    ("input_consume_cb", 0x1D760C, 0xE0),
    ("Update_Halalium_Hooks", 0x1D7A10, 0x4B0),
    ("LateUpdate_cb", 0x1D7EC4, 0x340),
    ("secondary_hook_cb", 0x1D81FC, 0x200),
    ("tertiary_hook_cb", 0x1D8404, 0x100),
    ("extraA_cb", 0x1D82A0, 0x130),
    ("extraB_cb", 0x1D83CC, 0x40),
    ("Bypass_getrr", 0x1D90B8, 0x100),
    ("libunity_base_resolve", 0x1D6AFC, 0x1C0),
    ("menu_body", 0x1DB874, 0x1600),
    ("wm_click_site", 0x1DB464, 0x200),
    ("track_hook_helper", 0x1D917C, 0x80),
    ("SkinChanger", 0x1D9E00, 0x1B4),
]


def elf_segments(data: bytes):
    e_phoff = struct.unpack_from("<Q", data, 32)[0]
    e_phentsize = struct.unpack_from("<H", data, 54)[0]
    e_phnum = struct.unpack_from("<H", data, 56)[0]
    segs = []
    for i in range(e_phnum):
        o = e_phoff + i * e_phentsize
        p_type = struct.unpack_from("<I", data, o)[0]
        p_offset, p_vaddr, _, p_filesz, p_memsz, _ = struct.unpack_from("<QQQQQQ", data, o + 8)
        if p_type == 1:
            segs.append((p_offset, p_vaddr, p_filesz, p_memsz))
    return segs


def main() -> None:
    so = SO_PATH.read_bytes()
    segs = elf_segments(so)

    def va_to_off(va: int):
        for off, vaddr, filesz, memsz in segs:
            if vaddr <= va < vaddr + memsz:
                rel = va - vaddr
                return off + rel if rel < filesz else None
        return None

    def read_cstr(va: int, maxlen: int = 96):
        off = va_to_off(va)
        if off is None:
            return None
        end = so.find(b"\x00", off, off + maxlen)
        if end < 0:
            return None
        raw = so[off:end]
        if not raw or not all(32 <= b < 127 for b in raw):
            return None
        return raw.decode("ascii")

    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    md.detail = True
    OUT.mkdir(parents=True, exist_ok=True)
    summary = []

    for name, va, size in FUNCS:
        off = va_to_off(va)
        assert off is not None, name
        insns = list(md.disasm(so[off : off + size], va))
        lines_a = [f"{i.address:08x}: {i.mnemonic:8} {i.op_str}" for i in insns]

        fields: dict[int, int] = {}
        strings: list[str] = []
        calls = []
        adrp_page: dict[str, int] = {}
        pending: dict[str, int] = {}
        full_rvas = []

        for i in insns:
            mnem = i.mnemonic
            if mnem == "adrp":
                dst = i.op_str.split(",")[0].strip()
                for op in i.operands:
                    if op.type == ARM64_OP_IMM:
                        adrp_page[dst] = op.imm & ~0xFFF
            elif mnem == "add":
                parts = [p.strip() for p in i.op_str.split(",")]
                if len(parts) >= 3 and parts[1] in adrp_page and parts[2].startswith("#"):
                    try:
                        imm = int(parts[2][1:], 0)
                    except ValueError:
                        imm = None
                    if imm is not None:
                        s = read_cstr(adrp_page[parts[1]] + imm)
                        if s and s not in strings:
                            strings.append(s)
            elif mnem in ("ldr", "ldrb", "ldrh", "str", "strb", "stur"):
                m = re.search(r"\[([^,]+),\s*#(0x[0-9a-fA-F]+|\d+)\]", i.op_str)
                if m:
                    imm = int(m.group(2), 0)
                    if 0x8 <= imm <= 0x400:
                        fields[imm] = fields.get(imm, 0) + 1
            elif mnem in ("mov", "movz"):
                m = re.match(r"(w\d+|x\d+),\s*#(0x[0-9a-fA-F]+|\d+)", i.op_str)
                if m:
                    pending[m.group(1)] = int(m.group(2), 0)
            elif mnem == "movk":
                m = re.match(r"(w\d+|x\d+),\s*#(0x[0-9a-fA-F]+|\d+),\s*lsl\s*#(\d+)", i.op_str)
                if m:
                    reg, imm, sh = m.group(1), int(m.group(2), 0), int(m.group(3))
                    base = pending.get(reg, 0)
                    pending[reg] = (base & ~(((1 << 16) - 1) << sh)) | (imm << sh)
                    val = pending[reg]
                    if 0x500000 <= val <= 0xA000000:
                        full_rvas.append({"reg": reg, "rva": hex(val), "at": hex(i.address)})
            elif mnem in ("bl", "blr"):
                tgt = None
                if mnem == "bl":
                    m = re.search(r"#(0x[0-9a-fA-F]+)", i.op_str)
                    if m:
                        tgt = int(m.group(1), 16)
                calls.append({"at": hex(i.address), "kind": mnem, "target": hex(tgt) if tgt else i.op_str})

        if name == "menu_body":
            clean = [s for s in strings if s in ALLOW_UI or s.startswith("##")]
        else:
            clean = []
            for s in strings:
                if s in ALLOW_UI or s.startswith("##") or s.startswith("Halalium") or "egl" in s.lower() or "lib" in s:
                    clean.append(s)
                elif any(k in s for k in ("ping", "Instantiate", "InputConsumer", "SwapBuffers", "getrr", "bypas")):
                    clean.append(s)

        field_rows = "\n".join(
            f"| `{hex(k)}` | {FIELD_HINTS.get(k, 'unknown')} | {v} |"
            for k, v in sorted(fields.items())
        ) or "| — | — | — |"
        rva_lines = "\n".join(f"- `{r['reg']}` = `{r['rva']}` @ `{r['at']}`" for r in full_rvas) or "_none_"
        str_lines = "\n".join(f"- `{s}`" for s in clean) or "_none_"
        call_lines = "\n".join(f"- `{c['at']}` → `{c['target']}` ({c['kind']})" for c in calls[:40]) or "_none_"

        (OUT / f"{name}.md").write_text(
            f"""# {name}

- VA: `{hex(va)}`
- Size scanned: `{hex(size)}`
- Pass A instructions: {len(insns)}
- Method: **2-command decompile** — (1) Capstone Pass A full disasm (2) Capstone Pass B field/RVA/string recovery

## Pass B — field LDR/STR map

| Offset | Meaning | Count |
|---|---|---|
{field_rows}

## Pass B — game RVAs (mov+movk)

{rva_lines}

## Pass B — strings (ADRP+ADD resolved only)

{str_lines}

## Pass B — calls (first 40)

{call_lines}

## Pass A — full disasm

```
{chr(10).join(lines_a)}
```
"""
        )
        (OUT / f"{name}.json").write_text(
            json.dumps(
                {
                    "name": name,
                    "va": hex(va),
                    "size": hex(size),
                    "insn": len(insns),
                    "fields": {hex(k): v for k, v in sorted(fields.items())},
                    "rvas": full_rvas,
                    "strings": clean,
                    "calls": calls[:80],
                },
                indent=2,
            )
        )
        summary.append({"name": name, "va": hex(va), "fields": len(fields), "rvas": full_rvas, "strings": clean})

    (OUT / "summary.json").write_text(json.dumps(summary, indent=2))
    index = [
        "# Halalium decompile index (2-pass)\n",
        "Each function: **Pass A** = full Capstone disasm, **Pass B** = field map + mov/movk RVAs + ADRP+ADD strings.\n",
        "Re-run: `python3 tools/halalium_emu/decompile_funcs.py`\n",
    ]
    for s in summary:
        rva_s = ", ".join(f'`{r["rva"]}`' for r in s["rvas"][:4]) or "—"
        index.append(
            f'- [{s["name"]}]({s["name"]}.md) @ `{s["va"]}` — fields={s["fields"]}, RVAs=[{rva_s}], strings={len(s["strings"])}'
        )
    (OUT / "INDEX.md").write_text("\n".join(index) + "\n")
    print(f"Wrote {len(summary)} functions → {OUT}")


if __name__ == "__main__":
    main()
