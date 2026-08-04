#!/usr/bin/env python3
"""Fix string xrefs, resolve PLT, recover globals, deepen key-func analysis."""
from __future__ import annotations

import json
import re
import struct
from collections import defaultdict
from pathlib import Path

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

SO = Path("/workspace/re/bin/libhalalium.so")
OUT = Path("/workspace/re")


def u64(b, o):
    return struct.unpack_from("<Q", b, o)[0]


def main():
    data = SO.read_bytes()
    with SO.open("rb") as f:
        elf = ELFFile(f)
        sections = {}
        for sec in elf.iter_sections():
            sections[sec.name] = {
                "addr": sec["sh_addr"],
                "off": sec["sh_offset"],
                "size": sec["sh_size"],
                "data": sec.data() if sec["sh_type"] != "SHT_NOBITS" else b"",
            }
        # PLT relocations -> symbol names
        plt_map = {}  # plt_entry_va -> name
        dynsym = elf.get_section_by_name(".dynsym")
        rela_plt = elf.get_section_by_name(".rela.plt")
        plt = sections.get(".plt")
        if rela_plt and dynsym and plt:
            # each rela.plt entry corresponds to plt slot starting after first reserved entry
            # aarch64: PLT[0] 32 bytes header, then 16-byte entries
            idx = 0
            for reloc in rela_plt.iter_relocations():
                sym = dynsym.get_symbol(reloc["r_info_sym"])
                # PLT entry VA
                entry = plt["addr"] + 32 + idx * 16
                plt_map[entry] = sym.name
                idx += 1

        # Also map GOT addresses from rela.plt r_offset
        got_map = {}
        for reloc in rela_plt.iter_relocations():
            sym = dynsym.get_symbol(reloc["r_info_sym"])
            got_map[reloc["r_offset"]] = sym.name

    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    md.detail = True
    text = sections[".text"]
    insns = list(md.disasm(text["data"], text["addr"]))

    # Build string table: va -> string (from .rodata primarily)
    ro = sections[".rodata"]
    str_at = {}
    i = 0
    d = ro["data"]
    while i < len(d):
        if 32 <= d[i] < 127:
            j = i
            while j < len(d) and 32 <= d[j] < 127:
                j += 1
            if j - i >= 3 and (j == len(d) or d[j] == 0):
                s = d[i:j].decode("ascii", errors="ignore")
                str_at[ro["addr"] + i] = s
            i = j + 1
        else:
            i += 1

    # ADRP+ADD absolute values at each PC
    pending = {}
    abs_at_pc = {}  # pc -> va materialized by add following adrp
    for insn in insns:
        m = re.match(r"adrp\s+(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)", insn.op_str)
        if m:
            pending[m.group(1)] = int(m.group(2), 0)
            continue
        m = re.match(
            r"add\s+(x\d+),\s*(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)",
            insn.op_str,
        )
        if m and m.group(1) == m.group(2) and m.group(1) in pending:
            va = pending[m.group(1)] + int(m.group(3), 0)
            abs_at_pc[insn.address] = {"reg": m.group(1), "va": va}
            continue
        # ADR literal
        if insn.mnemonic == "adr":
            m = re.match(r"(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)", insn.op_str)
            if m:
                abs_at_pc[insn.address] = {"reg": m.group(1), "va": int(m.group(2), 0)}

    # Xrefs: for each string VA, find PCs that materialize it
    xref = defaultdict(list)
    for pc, info in abs_at_pc.items():
        va = info["va"]
        if va in str_at:
            xref[str_at[va]].append({"pc": hex(pc), "va": hex(va), "reg": info["reg"]})

    # Resolve BL to PLT
    bl_imports = defaultdict(list)  # import_name -> [caller_pcs]
    bl_internal = defaultdict(list)
    for insn in insns:
        if insn.mnemonic != "bl":
            continue
        try:
            t = int(insn.op_str.replace("#", ""), 0)
        except Exception:
            continue
        if t in plt_map:
            bl_imports[plt_map[t]].append(hex(insn.address))
        else:
            bl_internal[hex(t)].append(hex(insn.address))

    # DobbyHook call sites = BL to PLT? or internal DobbyHook export
    # DobbyHook is exported at 0x2389d0 - also may be called via 0x26c560 wrapper
    # From egl_install: bl #0x26c560 after setting args — resolve that PLT slot
    interesting_wrappers = {}
    for entry, name in sorted(plt_map.items()):
        interesting_wrappers[hex(entry)] = name

    # Find what 0x26c550 / 0x26c560 / 0x26c540 / 0x26c590 are
    wrappers_used = [0x26C550, 0x26C560, 0x26C540, 0x26C590, 0x26C530, 0x26C330, 0x26C340, 0x26C350]
    wrapper_names = {hex(a): plt_map.get(a, "internal/unknown") for a in wrappers_used}

    # Recover BSS/data globals referenced from key functions via ADRP #0x279000
    globals_279 = defaultdict(set)
    for insn in insns:
        if "0x279000" in insn.op_str or insn.op_str.endswith("0x279000"):
            pass
    # scan ldr/str [xN, #imm] where xN was loaded from 0x279000 page
    # simpler: collect all unique offsets from adrp x?, #0x279000 then ldr/str
    page_reg = {}
    for insn in insns:
        m = re.match(r"adrp\s+(x\d+),\s*#0x279000", insn.op_str)
        if m:
            page_reg[m.group(1)] = insn.address
            continue
        m = re.match(
            r"(ldr|ldrb|str|strb)\s+(\w+),\s*\[(x\d+),\s*#(0x[0-9a-fA-F]+|\d+)\]",
            insn.op_str,
        )
        if m and m.group(3) in page_reg:
            off = int(m.group(4), 0)
            globals_279[hex(0x279000 + off)].add(f"{m.group(1)}@{hex(insn.address)}")
        m = re.match(r"(ldr|ldrb|str|strb)\s+(\w+),\s*\[(x\d+)\]$", insn.op_str)
        # skip

    # Also adr to 0x279xxx
    for pc, info in abs_at_pc.items():
        if 0x279000 <= info["va"] < 0x27B000:
            globals_279[hex(info["va"])].add(f"adr@{hex(pc)}")

    # Key string xrefs report
    needles = [
        "libEGL.so",
        "eglSwapBuffers",
        "Halalium_Hooks",
        "Halalium_Bypass",
        "t.me/lemminghack, 0.39.2",
        "Lemming",
        "##watermark",
        "##wm_click",
        "##settings_watermark",
        "##rage_left",
        "##skins_panel",
        "Enable Esp",
        "Silent Aim",
        "padla",
        "fresnel",
        "shador",
        "/sdcard/Android/data/com.axlebolt.standoff2/files/padla",
        "bypas hok result %d",
        "Skin Changer: Swapped to weapon %d (skin %d)",
        "Dear ImGui 1.92.7 (19270)",
        "Local Chams",
        "Enemy Chams",
        "Inf Ammo",
        "Through Walls",
        "Wallshot",
        "Auto Wall",
        "No spread",
        "Auto Fire",
        "Anti Aim",
        "Third Person",
        "scope fov",
        "Fov Check",
        "Health Bar",
    ]
    key_xrefs = {}
    for n in needles:
        # exact or substring
        hits = []
        for s, pcs in xref.items():
            if n == s or n in s:
                hits.append({"string": s, "refs": pcs})
        key_xrefs[n] = hits

    # Analyze egl_install fully with resolved imports
    egl_start, egl_end = 0x1D84CC, 0x1D8700
    egl_insns = [i for i in insns if egl_start <= i.address < egl_end]
    egl_trace = []
    for insn in egl_insns:
        note = ""
        if insn.mnemonic == "bl":
            try:
                t = int(insn.op_str.replace("#", ""), 0)
                note = f"  ; -> {plt_map.get(t, hex(t))}"
            except Exception:
                pass
        if insn.address in abs_at_pc:
            va = abs_at_pc[insn.address]["va"]
            if va in str_at:
                note += f"  ; str '{str_at[va][:60]}'"
            else:
                note += f"  ; imm {hex(va)}"
        # MOVZ/MOVK RVA reconstruction
        egl_trace.append(f"0x{insn.address:08x}: {insn.mnemonic:8s} {insn.op_str}{note}")

    (OUT / "disasm" / "egl_install_annotated.asm").write_text("\n".join(egl_trace) + "\n")

    # Same for Bypass, Update, JNI
    def annotate(start, end, path):
        lines = []
        for insn in insns:
            if not (start <= insn.address < end):
                continue
            note = ""
            if insn.mnemonic == "bl":
                try:
                    t = int(insn.op_str.replace("#", ""), 0)
                    note = f"  ; -> {plt_map.get(t, hex(t))}"
                except Exception:
                    pass
            if insn.mnemonic == "blr":
                note = "  ; indirect call"
            if insn.address in abs_at_pc:
                va = abs_at_pc[insn.address]["va"]
                if va in str_at:
                    note += f"  ; str '{str_at[va][:70]}'"
                elif 0x279000 <= va < 0x27C000:
                    note += f"  ; global {hex(va)}"
                else:
                    note += f"  ; imm {hex(va)}"
            # field offsets
            if "#0x" in insn.op_str or re.search(r"#\d+", insn.op_str):
                if insn.mnemonic.startswith("ldr") or insn.mnemonic.startswith("str"):
                    note += "  ; mem"
            lines.append(f"0x{insn.address:08x}: {insn.mnemonic:8s} {insn.op_str}{note}")
        Path(path).write_text("\n".join(lines) + "\n")
        return len(lines)

    annotate(0x1D69E4, 0x1D6B00, OUT / "disasm" / "JNI_OnLoad_annotated.asm")
    annotate(0x1D7A0C, 0x1D7E00, OUT / "disasm" / "Halalium_Hooks_Update_annotated.asm")
    annotate(0x1D7EC4, 0x1D8100, OUT / "disasm" / "Halalium_LateUpdate_annotated.asm")
    annotate(0x1D90B8, 0x1D9180, OUT / "disasm" / "Halalium_Bypass_annotated.asm")
    annotate(0x1D76F0, 0x1D7900, OUT / "disasm" / "egl_callback_annotated.asm")
    annotate(0x1D760C, 0x1D76F0, OUT / "disasm" / "input_callback_annotated.asm")

    # Reconstruct hook RVAs from egl_install MOVZ/MOVK
    # w23 = 0x8E7C40C etc.
    hook_table = [
        {"rva": "0x8E7C40C", "callback": "0x1d7a0c", "orig_slot": "0x2795a8", "role": "PlayerController.Update / Halalium_Hooks"},
        {"rva": "0x8E0085C", "callback": "0x1d81fc", "orig_slot": "0x2795d8", "role": "secondary"},
        {"rva": "0x79FE5E0", "alt_rva": "0x147E970", "callback": "0x1d8404", "role": "tertiary"},
        {"rva": "0x8E7CF50", "callback": "0x1d7ec4", "orig_slot": "?", "role": "LateUpdate (=Update+0xB44)"},
        {"rva": "0x8D663EC", "callback": "0x1d82a0", "role": "extra A"},
        {"rva": "0x8D2B2B0", "callback": "0x1d83cc", "role": "extra B"},
        {"symbol": "eglSwapBuffers", "callback": "0x1d76f0", "orig_slot": "0x279578", "role": "ImGui render"},
        {"symbol": "InputConsumer::consume", "callback": "0x1d760c", "orig_slot": "0x279580", "role": "touch/input"},
    ]

    # Import usage frequency
    import_freq = {k: len(v) for k, v in sorted(bl_imports.items(), key=lambda x: -len(x[1]))}

    # Globals annotation guesses from prior RE
    global_guess = {
        "0x279064": "menu_open_flag (byte) — toggled by ##wm_click",
        "0x2794f0": "libunity / game_base pointer",
        "0x2794f8": "feature/config object pointer",
        "0x279538": "esp/draw helper / enemy list ctx",
        "0x279578": "orig eglSwapBuffers",
        "0x279580": "orig InputConsumer::consume",
        "0x2795a8": "orig PlayerController.Update",
        "0x2795b0": "local PlayerController*",
        "0x2795b8": "last tick timestamp (ms/div)",
        "0x2795d8": "orig secondary hook",
        "0x279660": "periodic task / skin? buffer",
        "0x2796d8": "tracked hooks vector begin",
        "0x2796e0": "tracked hooks vector end (pair with +0x6d8)",
        "0x2796f0": "orig OnStart / getrr (bypass)",
    }

    out = {
        "plt_wrappers_used": wrapper_names,
        "plt_map_sample": {k: interesting_wrappers[k] for k in list(interesting_wrappers)[:30]},
        "import_call_freq": import_freq,
        "key_string_xrefs": key_xrefs,
        "globals_0x279xxx": {k: sorted(v)[:20] for k, v in sorted(globals_279.items())},
        "global_guess": global_guess,
        "hook_table": hook_table,
        "string_count_rodata": len(str_at),
        "abs_materializations": len(abs_at_pc),
    }
    (OUT / "extracted" / "deep_xrefs.json").write_text(json.dumps(out, indent=2) + "\n")
    (OUT / "extracted" / "plt_map.json").write_text(json.dumps(interesting_wrappers, indent=2) + "\n")
    (OUT / "extracted" / "import_freq.json").write_text(json.dumps(import_freq, indent=2) + "\n")

    # Print summary
    print("wrappers:", json.dumps(wrapper_names, indent=2))
    print("top imports:", list(import_freq.items())[:25])
    for n in ["libEGL.so", "eglSwapBuffers", "##wm_click", "Halalium_Bypass", "Enable Esp", "padla"]:
        print(n, "->", json.dumps(key_xrefs.get(n, []), indent=2)[:400])
    print("globals count", len(globals_279))


if __name__ == "__main__":
    main()
