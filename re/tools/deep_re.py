#!/usr/bin/env python3
"""Deep static RE of libhalalium.so (aarch64, stripped)."""
from __future__ import annotations

import json
import os
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

from capstone import CS_ARCH_ARM64, CS_MODE_ARM, Cs
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from elftools.elf.relocation import RelocationSection

SO = Path("/workspace/re/bin/libhalalium.so")
OUT = Path("/workspace/re")


def u32(b, off):
    return struct.unpack_from("<I", b, off)[0]


def u64(b, off):
    return struct.unpack_from("<Q", b, off)[0]


def load_elf():
    data = SO.read_bytes()
    with SO.open("rb") as f:
        elf = ELFFile(f)
        # Keep file handle open while reading sections into memory
        sections = {}
        for sec in elf.iter_sections():
            sections[sec.name] = {
                "addr": sec["sh_addr"],
                "off": sec["sh_offset"],
                "size": sec["sh_size"],
                "data": sec.data() if sec["sh_type"] != "SHT_NOBITS" else b"\x00" * sec["sh_size"],
            }
        # dynsyms
        dynsyms = []
        for sec in elf.iter_sections():
            if isinstance(sec, SymbolTableSection):
                for sym in sec.iter_symbols():
                    if sym.name:
                        dynsyms.append(
                            {
                                "name": sym.name,
                                "addr": sym["st_value"],
                                "size": sym["st_size"],
                                "bind": sym["st_info"]["bind"],
                                "type": sym["st_info"]["type"],
                                "shndx": sym["st_shndx"],
                            }
                        )
        # init/fini arrays
        init_array = []
        fini_array = []
        for name, lst in ((".init_array", init_array), (".fini_array", fini_array)):
            if name in sections and sections[name]["size"]:
                d = sections[name]["data"]
                for i in range(0, len(d), 8):
                    lst.append(u64(d, i))
        # dynamic needed
        dynamic = elf.get_section_by_name(".dynamic")
        needed = []
        soname = None
        if dynamic:
            for tag in dynamic.iter_tags():
                if tag.entry.d_tag == "DT_NEEDED":
                    needed.append(tag.needed)
                elif tag.entry.d_tag == "DT_SONAME":
                    soname = tag.soname
        build_id = None
        for sec in elf.iter_sections():
            if sec.name == ".note.gnu.build-id":
                # skip namesz, descsz, type, name
                raw = sec.data()
                namesz = u32(raw, 0)
                descsz = u32(raw, 4)
                name_pad = (namesz + 3) & ~3
                desc = raw[12 + name_pad : 12 + name_pad + descsz]
                build_id = desc.hex()
        # eh_frame_hdr FDE starts for function boundaries
        text = sections.get(".text")
        return {
            "data": data,
            "sections": sections,
            "dynsyms": dynsyms,
            "init_array": init_array,
            "fini_array": fini_array,
            "needed": needed,
            "soname": soname,
            "build_id": build_id,
            "text": text,
        }


def va_to_off(sections, va):
    for name, s in sections.items():
        if s["size"] == 0:
            continue
        if s["addr"] <= va < s["addr"] + s["size"]:
            return s["off"] + (va - s["addr"]), name
    return None, None


def find_strings(data, min_len=4):
    """Return list of (file_off, s)."""
    out = []
    i = 0
    n = len(data)
    while i < n:
        if 32 <= data[i] < 127:
            j = i
            while j < n and 32 <= data[j] < 127:
                j += 1
            if j - i >= min_len and (j == n or data[j] == 0):
                try:
                    s = data[i:j].decode("ascii")
                    out.append((i, s))
                except Exception:
                    pass
            i = j + 1
        else:
            i += 1
    return out


FEATURE_NEEDLES = [
    "Enable Esp",
    "Silent Aim",
    "Rage",
    "Anti Aim",
    "Anti Aim Pitch",
    "No spread",
    "Auto Fire",
    "Auto Wall",
    "Through Walls",
    "Wallshot",
    "Skin Changer",
    "Chams",
    "Local Chams",
    "Enemy Chams",
    "Spin",
    "Fov Check",
    "Fov Color",
    "Health Bar",
    "Bone",
    "Box",
    "Third Person",
    "Watermark",
    "scope fov",
    "Inf Ammo",
    "No Recoil",
    "Aimbot",
    "Trigger",
    "Bunny",
    "Speed",
    "God",
    "Fly",
    "Teleport",
]

INTERESTING = [
    "libEGL.so",
    "eglSwapBuffers",
    "DobbyHook",
    "DobbyDestroy",
    "DobbyCodePatch",
    "DobbySymbolResolver",
    "Halalium_Hooks",
    "Halalium_Bypass",
    "t.me/lemminghack",
    "0.39.2",
    "Lemming",
    "##watermark",
    "##wm_click",
    "##settings_watermark",
    "##skins_panel",
    "##skins_list",
    "##weapons_list",
    "##rage_left",
    "##rage_right_top",
    "##rage_right_bottom",
    "padla",
    "fresnel",
    "shador",
    "/sdcard/Android/data/com.axlebolt.standoff2",
    "JNI_OnLoad",
    "Skin Changer: Swapped",
    "bypas",
    "Dear ImGui",
    "imgui_impl_opengl3",
    "libhalalium.so",
    "Insert",
    "Right Alt",
    "RightAlt",
]


def disasm_range(md, data, sections, start_va, size, max_insns=8000):
    off, sec = va_to_off(sections, start_va)
    if off is None:
        return []
    chunk = data[off : off + size]
    out = []
    for i, insn in enumerate(md.disasm(chunk, start_va)):
        if i >= max_insns:
            break
        out.append(insn)
    return out


def insn_imm_addrs(insn):
    """Collect immediate addresses / PC-relative targets from insn."""
    addrs = []
    # adr/adrp/add patterns handled via operands text
    for op in insn.operands:
        if op.type == 2:  # IMM
            addrs.append(op.value.imm)
    # also parse ADRP+ADD manually from text if needed
    return addrs


def parse_adrp_add_pairs(insns):
    """Recover VA loads via ADRP xN, page; ADD xN, xN, #imm."""
    results = []  # (pc_of_add, dest_reg, va)
    pending = {}  # reg -> (page, adrp_pc)
    for insn in insns:
        m = re.match(r"adrp\s+(x\d+|xzr),\s*#(0x[0-9a-fA-F]+|-?\d+)", insn.op_str)
        if m:
            reg = m.group(1)
            page = int(m.group(2), 0)
            pending[reg] = (page, insn.address)
            continue
        m = re.match(
            r"add\s+(x\d+|xzr),\s*(x\d+|xzr),\s*#(0x[0-9a-fA-F]+|-?\d+)",
            insn.op_str,
        )
        if m and m.group(1) == m.group(2) and m.group(1) in pending:
            page, _ = pending[m.group(1)]
            imm = int(m.group(3), 0)
            results.append((insn.address, m.group(1), page + imm))
            continue
        m = re.match(
            r"ldr\s+(x\d+|w\d+),\s*\[(x\d+),\s*#(0x[0-9a-fA-F]+|-?\d+)\]",
            insn.op_str,
        )
        # leave for field offset collection
    return results


def collect_field_offsets(insns):
    """ldr/str [xn, #imm] offsets used in function."""
    offs = defaultdict(int)
    for insn in insns:
        if insn.mnemonic in ("ldr", "ldrb", "ldrh", "ldrsb", "ldrsh", "ldp", "str", "strb", "strh", "stp"):
            for m in re.finditer(r"#(0x[0-9a-fA-F]+|\d+)", insn.op_str):
                v = int(m.group(1), 0)
                if 0 < v < 0x1000:
                    offs[v] += 1
    return dict(sorted(offs.items(), key=lambda x: -x[1]))


def find_bl_targets(insns):
    targets = []
    for insn in insns:
        if insn.mnemonic == "bl":
            try:
                t = int(insn.op_str.replace("#", ""), 0)
                targets.append((insn.address, t))
            except Exception:
                pass
    return targets


def disasm_to_text(insns, limit=None):
    lines = []
    for i, insn in enumerate(insns):
        if limit is not None and i >= limit:
            lines.append(f"; ... truncated ({len(insns) - limit} more)")
            break
        lines.append(f"0x{insn.address:08x}:  {insn.mnemonic:8s} {insn.op_str}")
    return "\n".join(lines)


def rough_pseudo(insns, name):
    """Very rough pseudocode from control-flow + string comments."""
    lines = [f"// recovered pseudocode: {name}", f"void {name}(...) {{"]
    for insn in insns[:500]:
        if insn.mnemonic == "ret":
            lines.append("    return;")
        elif insn.mnemonic == "bl":
            lines.append(f"    call(0x{insn.op_str.replace('#','')});  // @0x{insn.address:x}")
        elif insn.mnemonic in ("cbz", "cbnz", "tbz", "tbnz", "b.eq", "b.ne", "b.lt", "b.gt", "b.le", "b.ge", "b"):
            lines.append(f"    // branch {insn.mnemonic} {insn.op_str}")
        elif insn.mnemonic == "adrp" or (
            insn.mnemonic == "add" and "x" in insn.op_str and "#" in insn.op_str
        ):
            pass
    lines.append("}")
    return "\n".join(lines)


def main():
    print("[*] loading ELF…")
    info = load_elf()
    data = info["data"]
    sections = info["sections"]
    text = info["text"]
    md = Cs(CS_ARCH_ARM64, CS_MODE_ARM)
    md.detail = True

    # --- strings ---
    print("[*] scanning strings…")
    all_strs = find_strings(data, 4)
    # map file_off -> va for loadable segments approx via sections
    str_index = {}  # exact string -> list of vas
    for off, s in all_strs:
        va = None
        for name, sec in sections.items():
            if sec["off"] <= off < sec["off"] + max(sec["size"], 1) and name != ".bss":
                # for NOBITS size may not match file
                if sec["data"] and off < sec["off"] + len(sec["data"]):
                    va = sec["addr"] + (off - sec["off"])
                    break
        if va is None:
            # fallback: assume identity for file-backed
            for name, sec in sections.items():
                if sec["off"] <= off < sec["off"] + len(sec.get("data") or b""):
                    va = sec["addr"] + (off - sec["off"])
                    break
        if va is not None:
            str_index.setdefault(s, []).append({"off": off, "va": va})

    # interesting / features
    interesting_hits = {}
    for needle in INTERESTING + FEATURE_NEEDLES:
        hits = []
        for s, locs in str_index.items():
            if needle in s:
                hits.append({"string": s, "locs": locs})
        if hits:
            interesting_hits[needle] = hits

    features_found = sorted(
        {n for n in FEATURE_NEEDLES if n in interesting_hits or any(n in s for s in str_index)}
    )
    # also collect any ImGui ## ids
    imgui_ids = sorted({s for s in str_index if s.startswith("##")})
    log_fmt = sorted({s for s in str_index if "%d" in s or "%s" in s or "%p" in s})[:200]

    # --- disassemble full .text once for xrefs ---
    print("[*] disassembling .text…")
    text_insns = list(md.disasm(text["data"], text["addr"]))
    print(f"    {len(text_insns)} instructions")

    # ADRP+ADD materializations
    print("[*] recovering ADRP/ADD absolute refs…")
    abs_refs = parse_adrp_add_pairs(text_insns)  # (pc, reg, va)
    # map target_va -> list of pcs that materialize it
    xref_to = defaultdict(list)
    for pc, reg, va in abs_refs:
        xref_to[va].append({"pc": pc, "reg": reg})

    # Also BL targets as call graph
    print("[*] building call graph…")
    callers = defaultdict(list)
    callees = defaultdict(list)
    for insn in text_insns:
        if insn.mnemonic == "bl":
            try:
                t = int(insn.op_str.replace("#", ""), 0)
                callers[t].append(insn.address)
                callees[insn.address].append(t)
            except Exception:
                pass

    # function starts heuristic: init_array + exported T + frequent BL targets + eh-ish
    func_starts = set()
    for a in info["init_array"]:
        if text["addr"] <= a < text["addr"] + text["size"]:
            func_starts.add(a)
    for sym in info["dynsyms"]:
        if sym["type"] == "STT_FUNC" and sym["addr"]:
            if text["addr"] <= sym["addr"] < text["addr"] + text["size"]:
                func_starts.add(sym["addr"])
    # hot BL targets
    for t, cs in callers.items():
        if len(cs) >= 2 and text["addr"] <= t < text["addr"] + text["size"]:
            func_starts.add(t)
    # after RET + padding align
    prev_ret = False
    for insn in text_insns:
        if prev_ret and insn.address % 4 == 0:
            # likely new function if not mid-literal
            if insn.mnemonic not in ("udf", ".byte"):
                func_starts.add(insn.address)
        prev_ret = insn.mnemonic == "ret"

    func_starts = sorted(func_starts)
    print(f"    ~{len(func_starts)} candidate functions")

    def next_func_end(start):
        for s in func_starts:
            if s > start:
                return min(s, start + 0x4000)
        return min(text["addr"] + text["size"], start + 0x4000)

    # Key known addresses from prior RE + JNI
    KEY_FUNCS = {
        "JNI_OnLoad": 0x1D69E4,
        "egl_callback": 0x1D76F0,
        "input_callback": 0x1D760C,
        "Halalium_Hooks_Update": 0x1D7A0C,
        "Halalium_LateUpdate": 0x1D7EC4,
        "egl_install": 0x1D84CC,
        "Halalium_Bypass": 0x1D90B8,
        "secondary_hook_cb": 0x1D81FC,
        "tertiary_hook_cb": 0x1D8404,
        "extra_hook_A": 0x1D82A0,
        "extra_hook_B": 0x1D83CC,
        "DobbyHook": 0x2389D0,
        "DobbyDestroy": 0x23B680,
        "DobbyGetVersion": 0x23B674,
        "DobbyCodePatch": 0x23BCF0,
        "DobbySymbolResolver": 0x23C5D8,
    }

    # String xrefs for key strings
    print("[*] computing string xrefs…")
    string_xrefs = {}
    for needle, hits in interesting_hits.items():
        pcs = []
        for h in hits:
            for loc in h["locs"]:
                va = loc["va"]
                # exact + nearby (string ptr often to start)
                for delta in range(0, 1):
                    for ref in xref_to.get(va + delta, []):
                        pcs.append({"pc": ref["pc"], "str_va": va, "string": h["string"], "reg": ref["reg"]})
        string_xrefs[needle] = pcs

    # Disassemble key functions
    print("[*] dumping key functions…")
    key_disasm = {}
    key_meta = {}
    for name, addr in KEY_FUNCS.items():
        end = next_func_end(addr)
        # prefer size from dynsym if present
        for sym in info["dynsyms"]:
            if sym["addr"] == addr and sym["size"]:
                end = addr + sym["size"]
                break
        size = max(0x40, min(end - addr, 0x3000))
        insns = disasm_range(md, data, sections, addr, size)
        # trim at first ret that looks like end if huge — keep all for analysis file
        key_disasm[name] = disasm_to_text(insns, limit=400)
        fields = collect_field_offsets(insns)
        bls = find_bl_targets(insns)
        abs_in = [(pc, reg, va) for pc, reg, va in parse_adrp_add_pairs(insns)]
        # which strings does this func touch?
        touched = []
        for pc, reg, va in abs_in:
            for s, locs in str_index.items():
                for loc in locs:
                    if loc["va"] == va:
                        touched.append({"pc": pc, "string": s[:120]})
        key_meta[name] = {
            "addr": hex(addr),
            "size_analyzed": size,
            "insn_count": len(insns),
            "field_offsets": fields,
            "bl_targets": [{"from": hex(a), "to": hex(b)} for a, b in bls[:80]],
            "abs_refs": [{"pc": hex(pc), "reg": reg, "va": hex(va)} for pc, reg, va in abs_in[:80]],
            "strings": touched[:40],
            "callers": [hex(c) for c in callers.get(addr, [])[:40]],
        }
        # write individual asm
        (OUT / "disasm" / f"{name}.asm").write_text(disasm_to_text(insns) + "\n")
        (OUT / "decompile" / f"{name}.md").write_text(
            f"# {name}\n\n- VA: `{hex(addr)}`\n- insns: {len(insns)}\n\n"
            f"## Field offsets (ldr/str immediates)\n\n```\n{json.dumps(fields, indent=2)}\n```\n\n"
            f"## Strings referenced\n\n```\n{json.dumps(touched[:40], indent=2)}\n```\n\n"
            f"## BL targets\n\n```\n{json.dumps(key_meta[name]['bl_targets'][:40], indent=2)}\n```\n\n"
            f"## Rough pseudocode\n\n```c\n{rough_pseudo(insns, name)}\n```\n"
        )

    # Scan for DobbyHook call sites and nearby ADRP literals (hook RVAs)
    print("[*] locating DobbyHook call sites…")
    dobby_sites = []
    for insn in text_insns:
        if insn.mnemonic == "bl":
            try:
                t = int(insn.op_str.replace("#", ""), 0)
            except Exception:
                continue
            if t == KEY_FUNCS["DobbyHook"]:
                # look back ~30 insns for mov/adr immediates that look like RVAs or callbacks
                dobby_sites.append(insn.address)

    # INIT_ARRAY constructors disasm
    init_dump = []
    for a in info["init_array"]:
        if not (text["addr"] <= a < text["addr"] + text["size"]):
            init_dump.append({"addr": hex(a), "note": "outside .text"})
            continue
        insns = disasm_range(md, data, sections, a, 0x200)
        init_dump.append(
            {
                "addr": hex(a),
                "insn_count": len(insns),
                "preview": disasm_to_text(insns, limit=40),
            }
        )
        (OUT / "disasm" / f"init_{a:x}.asm").write_text(disasm_to_text(insns) + "\n")

    # JNI_OnLoad deeper: look for RegisterNatives / pthread_create / dlopen patterns via BL
    jni_bls = key_meta["JNI_OnLoad"]["bl_targets"]

    # Collect ALL feature-like UI strings more broadly
    ui_strings = sorted(
        s
        for s in str_index
        if any(
            k in s.lower()
            for k in (
                "esp",
                "aim",
                "chams",
                "skin",
                "rage",
                "fov",
                "wall",
                "recoil",
                "spread",
                "ammo",
                "spin",
                "silent",
                "watermark",
                "bypass",
                "halalium",
                "lemming",
            )
        )
        and len(s) < 80
    )

    # Exports summary
    exports = [
        s
        for s in info["dynsyms"]
        if s["bind"] == "STB_GLOBAL" and s["type"] == "STT_FUNC" and s["addr"] and s["shndx"] != "SHN_UNDEF"
    ]
    imports = [s for s in info["dynsyms"] if s["shndx"] == "SHN_UNDEF"]

    # Write JSON artifacts
    profile = {
        "so": str(SO),
        "soname": info["soname"],
        "build_id": info["build_id"],
        "size": len(data),
        "arch": "aarch64",
        "stripped": True,
        "needed": info["needed"],
        "text": {"addr": hex(text["addr"]), "size": hex(text["size"])},
        "init_array": [hex(a) for a in info["init_array"]],
        "fini_array": [hex(a) for a in info["fini_array"]],
        "version_string": "t.me/lemminghack, 0.39.2",
        "target_package": "com.axlebolt.standoff2",
        "features": features_found,
        "imgui_ids": imgui_ids,
        "ui_strings": ui_strings,
        "key_functions": {k: v["addr"] for k, v in key_meta.items()},
        "dobby_hook_sites": [hex(a) for a in dobby_sites],
        "exports_count": len(exports),
        "imports_count": len(imports),
        "candidate_functions": len(func_starts),
        "instruction_count": len(text_insns),
    }

    (OUT / "extracted" / "profile.json").write_text(json.dumps(profile, indent=2) + "\n")
    (OUT / "extracted" / "key_functions.json").write_text(json.dumps(key_meta, indent=2) + "\n")
    (OUT / "extracted" / "string_xrefs.json").write_text(
        json.dumps(string_xrefs, indent=2, default=str) + "\n"
    )
    (OUT / "extracted" / "interesting_strings.json").write_text(
        json.dumps(interesting_hits, indent=2) + "\n"
    )
    (OUT / "extracted" / "init_array.json").write_text(json.dumps(init_dump, indent=2) + "\n")
    (OUT / "extracted" / "exports.json").write_text(
        json.dumps(
            [{"name": e["name"], "addr": hex(e["addr"]), "size": e["size"]} for e in exports],
            indent=2,
        )
        + "\n"
    )
    (OUT / "extracted" / "imports.json").write_text(
        json.dumps([{"name": i["name"]} for i in imports], indent=2) + "\n"
    )
    (OUT / "extracted" / "dobby_sites.json").write_text(
        json.dumps([hex(a) for a in dobby_sites], indent=2) + "\n"
    )
    (OUT / "extracted" / "log_formats.json").write_text(json.dumps(log_fmt, indent=2) + "\n")

    # func map: top called functions
    top_called = sorted(callers.items(), key=lambda x: -len(x[1]))[:200]
    (OUT / "extracted" / "func_map.json").write_text(
        json.dumps(
            {
                "top_called": [{"addr": hex(a), "callers": len(cs)} for a, cs in top_called],
                "key": KEY_FUNCS,
            },
            indent=2,
        )
        + "\n"
    )

    print("[+] done")
    print(json.dumps({k: profile[k] for k in ("build_id", "features", "dobby_hook_sites", "candidate_functions", "instruction_count")}, indent=2))


if __name__ == "__main__":
    main()
