#!/usr/bin/env python3
"""
Halalium emulator / RE toolkit for Melodium updates.

Goal: stop hand-updating Melodium every game patch.
Drop a new libhalalium.so (+ dump.cs / script.json) and regenerate the
offset/hook/feature profile Melodium consumes.

Usage:
  python3 tools/halalium_emu/halalium_emu.py profile \\
      --so halalium/bin/libhalalium.so \\
      --script okak/okaakka/script.json \\
      --dump okak/okaakka/dump \\
      --out tools/halalium_emu/out

  python3 tools/halalium_emu/halalium_emu.py apply \\
      --profile tools/halalium_emu/out/profile.json

  python3 tools/halalium_emu/halalium_emu.py diff \\
      --old old/libhalalium.so --new new/libhalalium.so
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

ROOT = Path(__file__).resolve().parents[2]

# ---------------------------------------------------------------------------
# Known Halalium surface (from prior RE). Emulator looks for these in every SO.
# ---------------------------------------------------------------------------
KNOWN_STRINGS = {
    "egl_lib": b"libEGL.so",
    "egl_swap": b"eglSwapBuffers",
    "watermark": b"##watermark",
    "wm_click": b"##wm_click",
    "settings_wm": b"##settings_watermark",
    "brand": b"Lemming",
    "insert": b"Insert",
    "right_alt": b"RightAlt",
    "dobby_hook": b"DobbyHook",
    "hooks_thread": b"Halalium_Hooks",
    "bypass_thread": b"Halalium_Bypass",
    "padla": b"padla",
    "fresnel": b"fresnel",
    "shador": b"shador",
}

FEATURE_STRINGS = [
    "Enable Esp",
    "Silent Aim",
    "Rage",
    "Anti Aim",
    "No spread",
    "Auto Fire",
    "Auto Wall",
    "Through Walls",
    "Wallshot",
    "Skin Changer",
    "Chams",
    "Local Chams",
    "Spin",
    "Fov Check",
    "Health Bar",
    "Bone",
    "Box",
    "Third Person",
    "Watermark",
]

# Fields Halalium LDR-confirmed; used as Melodium ground truth defaults.
CONFIRMED_FIELDS = {
    "Player.photon_player": 0x160,
    "Player.weaponry_controller": 0x88,
    "Weaponry.weapon_controller": 0xA0,
    "Player.occlusion_controller": 0xB8,
    "Player.main_camera": 0xE8,
    "Player.team": 0x79,
    "GameController.player_controls": 0x2B0,
    "Il2Cpp.klass_static_fields": 0x90,
}

TYPEINFO_KEYS = {
    "PlayerManager": "PlayerManager_TypeInfo",
    "GameController": "GameController_TypeInfo",
    "PhotonNetwork": "PhotonNetwork_TypeInfo",
    "BombManager": "BombManager_TypeInfo",
    "InventoryManager": "InventoryManager_TypeInfo",
    "PlayerControls": "PlayerControls_TypeInfo",
    "PlayerController": "PlayerController_TypeInfo",
    "WeaponController": "WeaponController_TypeInfo",
    "WeaponManager": "WeaponManager_TypeInfo",
    "GameManager": "GameManager_TypeInfo",
    "TouchController": "TouchController_TypeInfo",
    "AntiCheatManager": "AntiCheatManager_TypeInfo",
}


@dataclass
class ElfSection:
    name: str
    addr: int
    offset: int
    size: int


@dataclass
class Profile:
    so_path: str
    build_id: str = ""
    version_string: str = ""
    strings: Dict[str, Any] = field(default_factory=dict)
    features: List[str] = field(default_factory=list)
    hooks: Dict[str, Any] = field(default_factory=dict)
    typeinfo: Dict[str, int] = field(default_factory=dict)
    fields: Dict[str, int] = field(default_factory=dict)
    notes: List[str] = field(default_factory=list)


def parse_elf_sections(data: bytes) -> Dict[str, ElfSection]:
    if data[:4] != b"\x7fELF":
        raise ValueError("not an ELF")
    e_shoff = struct.unpack_from("<Q", data, 0x28)[0]
    e_shentsize = struct.unpack_from("<H", data, 0x3A)[0]
    e_shnum = struct.unpack_from("<H", data, 0x3C)[0]
    e_shstrndx = struct.unpack_from("<H", data, 0x3E)[0]
    shstr = e_shoff + e_shstrndx * e_shentsize
    shstr_off = struct.unpack_from("<Q", data, shstr + 0x18)[0]
    out: Dict[str, ElfSection] = {}
    for i in range(e_shnum):
        sh = e_shoff + i * e_shentsize
        name_off = struct.unpack_from("<I", data, sh)[0]
        name = data[shstr_off + name_off :].split(b"\0", 1)[0].decode(errors="replace")
        addr = struct.unpack_from("<Q", data, sh + 0x10)[0]
        offset = struct.unpack_from("<Q", data, sh + 0x18)[0]
        size = struct.unpack_from("<Q", data, sh + 0x20)[0]
        out[name] = ElfSection(name, addr, offset, size)
    return out


def read_build_id(data: bytes, sections: Dict[str, ElfSection]) -> str:
    note = sections.get(".note.gnu.build-id")
    if not note:
        return ""
    # skip Nhdr
    blob = data[note.offset : note.offset + note.size]
    if len(blob) < 16:
        return ""
    namesz, descsz, ntype = struct.unpack_from("<III", blob, 0)
    desc_off = 12 + ((namesz + 3) & ~3)
    return blob[desc_off : desc_off + descsz].hex()


def find_string(data: bytes, needle: bytes) -> Optional[int]:
    i = data.find(needle + b"\0")
    if i < 0:
        i = data.find(needle)
    return i if i >= 0 else None


def decode_adrp(insn: int, pc: int) -> Optional[Tuple[int, int]]:
    if (insn & 0x9F000000) != 0x90000000:
        return None
    rd = insn & 0x1F
    immlo = (insn >> 29) & 0x3
    immhi = (insn >> 5) & 0x7FFFF
    imm = (immhi << 2) | immlo
    if imm & (1 << 20):
        imm |= ~((1 << 21) - 1)
    page = (pc & ~0xFFF) + (imm << 12)
    return rd, page


def decode_add_imm(insn: int) -> Optional[Tuple[int, int, int]]:
    if (insn & 0xFF800000) != 0x91000000:
        return None
    rd = insn & 0x1F
    rn = (insn >> 5) & 0x1F
    imm12 = (insn >> 10) & 0xFFF
    shift = (insn >> 22) & 0x3
    if shift == 1:
        imm12 <<= 12
    elif shift != 0:
        return None
    return rd, rn, imm12


def find_string_xrefs(data: bytes, sections: Dict[str, ElfSection], target_va: int) -> List[int]:
    text = sections.get(".text")
    if not text:
        return []
    page = target_va & ~0xFFF
    off12 = target_va & 0xFFF
    hits: List[int] = []
    for i in range(0, text.size - 8, 4):
        pc = text.addr + i
        insn = struct.unpack_from("<I", data, text.offset + i)[0]
        adrp = decode_adrp(insn, pc)
        if not adrp:
            continue
        rd, pg = adrp
        if pg != page:
            continue
        for j in range(1, 8):
            a = struct.unpack_from("<I", data, text.offset + i + j * 4)[0]
            add = decode_add_imm(a)
            if not add:
                continue
            ard, arn, imm = add
            if arn == rd and imm == off12:
                hits.append(pc)
                break
    return hits


def va_of_file_off(sections: Dict[str, ElfSection], file_off: int) -> Optional[int]:
    for s in sections.values():
        if s.offset <= file_off < s.offset + s.size:
            return s.addr + (file_off - s.offset)
    return None


def extract_version(data: bytes) -> str:
    # e.g. t.me/lemminghack, 0.39.2
    m = re.search(rb"t\.me/[a-zA-Z0-9_]+,\s*([0-9]+\.[0-9]+\.[0-9]+)", data)
    if m:
        return m.group(0).decode(errors="replace")
    m = re.search(rb"0\.\d+\.\d+", data)
    return m.group(0).decode() if m else ""


def load_typeinfo_from_script(script_path: Path) -> Dict[str, int]:
    raw = json.loads(script_path.read_text(encoding="utf-8", errors="replace"))
    entries: List[Tuple[str, int]] = []

    def add_name_addr(name: Any, addr: Any) -> None:
        if not isinstance(name, str):
            return
        try:
            if isinstance(addr, str):
                val = int(addr, 0)
            else:
                val = int(addr)
        except Exception:
            return
        entries.append((name, val))

    if isinstance(raw, dict):
        meta = raw.get("ScriptMetadata")
        if isinstance(meta, list):
            for item in meta:
                if isinstance(item, dict):
                    add_name_addr(item.get("Name"), item.get("Address"))
        for key, val in raw.items():
            if isinstance(val, int) and "TypeInfo" in str(key):
                add_name_addr(str(key), val)

    out: Dict[str, int] = {}
    for short, full in TYPEINFO_KEYS.items():
        # Prefer exact / fully-qualified end match. Never substring
        # (AimTrainingGameController_TypeInfo contains GameController_TypeInfo).
        exact = [a for n, a in entries if n == full]
        fq = [a for n, a in entries if n.endswith("." + full)]
        if exact:
            out[short] = exact[0]
        elif fq:
            # Prefer shortest FQ name (root class over nested noise)
            fq_names = [(n, a) for n, a in entries if n.endswith("." + full)]
            fq_names.sort(key=lambda t: len(t[0]))
            out[short] = fq_names[0][1]
    return out


def parse_dump_fields(dump_path: Path) -> Dict[str, int]:
    """Best-effort field scrape from dump.cs for PlayerController / related."""
    if not dump_path.exists():
        return {}
    text = dump_path.read_text(encoding="utf-8", errors="replace")
    out: Dict[str, int] = {}

    def grab(class_name: str, field_substr: str, key: str):
        # naive: find class block then first matching // 0xNN field
        m = re.search(rf"class {re.escape(class_name)}\b[^{{]*\{{(?P<body>.*?)\n\}}", text, re.S)
        if not m:
            return
        body = m.group("body")
        fm = re.search(
            rf"{re.escape(field_substr)}[^;\n]*;\s*//\s*(0x[0-9A-Fa-f]+)",
            body,
        )
        if fm:
            out[key] = int(fm.group(1), 16)

    grab("PlayerController", "PhotonPlayer", "Player.photon_player")
    grab("PlayerController", "WeaponryController", "Player.weaponry_controller")
    grab("PlayerController", "PlayerOcclusionController", "Player.occlusion_controller")
    grab("PlayerController", "PlayerMainCamera", "Player.main_camera")
    grab("PlayerController", "Team", "Player.team")
    grab("WeaponryController", "WeaponController", "Weaponry.weapon_controller")
    grab("GameController", "PlayerControls", "GameController.player_controls")
    return out


def analyze_hooks(data: bytes, sections: Dict[str, ElfSection], strings: Dict[str, Any]) -> Dict[str, Any]:
    hooks: Dict[str, Any] = {
        "render": {
            "strategy": "dlsym(libEGL.so, eglSwapBuffers) + DobbyHook",
            "melodium_strategy": "dlsym + GOT pointer-swap (same symbol)",
        }
    }
    egl_off = strings.get("egl_swap", {}).get("file_off")
    if egl_off is None:
        return hooks
    egl_va = va_of_file_off(sections, egl_off)
    if egl_va is None:
        return hooks
    xrefs = find_string_xrefs(data, sections, egl_va)
    hooks["render"]["egl_string_va"] = hex(egl_va)
    hooks["render"]["install_xrefs"] = [hex(x) for x in xrefs]
    if xrefs:
        # Halalium install uses ADR of hook callback shortly after xref
        hooks["render"]["install_pc"] = hex(xrefs[0])
        hooks["render"]["note"] = (
            "At install_pc: load libEGL.so + eglSwapBuffers, dlsym, then DobbyHook(callback, &orig)"
        )
    wm = strings.get("wm_click", {}).get("file_off")
    if wm is not None:
        wm_va = va_of_file_off(sections, wm)
        if wm_va is not None:
            wm_x = find_string_xrefs(data, sections, wm_va)
            hooks["menu_open"] = {
                "method": "InvisibleButton ##wm_click on ##watermark toggles open flag",
                "wm_click_va": hex(wm_va),
                "xrefs": [hex(x) for x in wm_x],
            }
    return hooks


def build_profile(so: Path, script: Optional[Path], dump: Optional[Path]) -> Profile:
    data = so.read_bytes()
    sections = parse_elf_sections(data)
    prof = Profile(so_path=str(so))
    prof.build_id = read_build_id(data, sections)
    prof.version_string = extract_version(data)
    prof.fields = dict(CONFIRMED_FIELDS)

    for key, needle in KNOWN_STRINGS.items():
        off = find_string(data, needle)
        entry: Dict[str, Any] = {"present": off is not None}
        if off is not None:
            entry["file_off"] = off
            va = va_of_file_off(sections, off)
            if va is not None:
                entry["va"] = hex(va)
        prof.strings[key] = entry

    for feat in FEATURE_STRINGS:
        if data.find(feat.encode()) >= 0:
            prof.features.append(feat)

    prof.hooks = analyze_hooks(data, sections, prof.strings)

    if script and script.exists():
        prof.typeinfo = load_typeinfo_from_script(script)
        if not prof.typeinfo:
            prof.notes.append("script.json parsed but no TypeInfo keys matched — check dump format")
    else:
        prof.notes.append("no script.json — TypeInfo not refreshed (keep previous Offsets)")

    if dump and dump.exists():
        scraped = parse_dump_fields(dump)
        prof.fields.update(scraped)
    else:
        prof.notes.append("no dump.cs — using Halalium-confirmed field defaults")

    # Emulator contract: Melodium must implement these
    prof.notes.append(
        "Melodium contract: eglSwapBuffers draw path + ##watermark/##wm_click open + Offsets from this profile"
    )
    return prof


def emit_offsets_header(prof: Profile, out_path: Path, ns: str = "OffsetsGenerated") -> None:
    ti = prof.typeinfo
    fields = prof.fields
    ver = prof.version_string or "unknown"

    def g(key: str, default: int) -> str:
        return f"0x{fields.get(key, default):X}"

    def t(key: str, default: int) -> str:
        return f"0x{ti.get(key, default):X}"

    body = f"""// AUTO-GENERATED by tools/halalium_emu/halalium_emu.py — do not edit by hand.
// Source SO: {prof.so_path}
// BuildID: {prof.build_id}
// Version: {ver}
#pragma once
#include <cstdint>

namespace {ns} {{

namespace Il2Cpp {{
    constexpr uintptr_t klass_static_fields = {g('Il2Cpp.klass_static_fields', 0x90)};
}}

namespace TypeInfo {{
    constexpr uintptr_t PlayerManager    = {t('PlayerManager', 0xAC5E190)};
    constexpr uintptr_t GameController   = {t('GameController', 0xAC58BB0)};
    constexpr uintptr_t PhotonNetwork    = {t('PhotonNetwork', 0xAC5DE18)};
    constexpr uintptr_t BombManager      = {t('BombManager', 0xAC4FAC0)};
    constexpr uintptr_t InventoryManager = {t('InventoryManager', 0xAC5C018)};
    constexpr uintptr_t PlayerControls   = {t('PlayerControls', 0xAC5E0E0)};
    constexpr uintptr_t PlayerController = {t('PlayerController', 0xAC5E0D8)};
    constexpr uintptr_t WeaponController = {t('WeaponController', 0xAC61A18)};
    constexpr uintptr_t WeaponManager    = {t('WeaponManager', 0xAC61A78)};
    constexpr uintptr_t GameManager      = {t('GameManager', 0xAC58C00)};
    constexpr uintptr_t TouchController  = {t('TouchController', 0xAC60B48)};
    constexpr uintptr_t AntiCheatManager = {t('AntiCheatManager', 0xAC4DA30)};
}}

namespace Player {{
    constexpr uintptr_t photon_player        = {g('Player.photon_player', 0x160)};
    constexpr uintptr_t weaponry_controller  = {g('Player.weaponry_controller', 0x88)};
    constexpr uintptr_t occlusion_controller = {g('Player.occlusion_controller', 0xB8)};
    constexpr uintptr_t main_camera          = {g('Player.main_camera', 0xE8)};
    constexpr uintptr_t team                 = {g('Player.team', 0x79)};
}}

namespace Weaponry {{
    constexpr uintptr_t weapon_controller = {g('Weaponry.weapon_controller', 0xA0)};
}}

namespace GameController {{
    constexpr uintptr_t player_controls = {g('GameController.player_controls', 0x2B0)};
}}

namespace Hook {{
    // Halalium: DobbyHook(dlsym(eglSwapBuffers))
    // Melodium: GOT swap of same symbol
    constexpr bool use_egl_swap_buffers = true;
    constexpr bool open_menu_via_watermark_click = true;
}}

}} // namespace {ns}
"""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(body, encoding="utf-8")


def emit_melodium_bridge(out_path: Path) -> None:
    """Thin include Melodium uses so live offsets track generated header."""
    out_path.write_text(
        """#pragma once
// Melodium ↔ Halalium emulator bridge.
// Prefer generated header when present; fall back to hand-curated 0.39.2 table.
#if __has_include("sdk/generated/Offsets_generated.h")
#include "sdk/generated/Offsets_generated.h"
namespace Offsets = OffsetsGenerated;
#elif __has_include("../../../melodium/sdk/Offsets_0.39.2.h")
#include "../../../melodium/sdk/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#else
#include "../../../halalium/sdk/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#endif
""",
        encoding="utf-8",
    )


def apply_profile(profile_path: Path) -> None:
    prof_data = json.loads(profile_path.read_text(encoding="utf-8"))
    out_dir = profile_path.parent
    # Reconstruct minimal Profile for header emit
    prof = Profile(so_path=prof_data.get("so_path", ""))
    prof.build_id = prof_data.get("build_id", "")
    prof.version_string = prof_data.get("version_string", "")
    prof.typeinfo = {k: int(v, 0) if isinstance(v, str) else int(v) for k, v in prof_data.get("typeinfo", {}).items()}
    prof.fields = {k: int(v, 0) if isinstance(v, str) else int(v) for k, v in prof_data.get("fields", {}).items()}

    gen = ROOT / "internal-main/internal-main/sdk/generated/Offsets_generated.h"
    emit_offsets_header(prof, gen)
    emit_offsets_header(prof, ROOT / "melodium/sdk/Offsets_generated.h")
    emit_offsets_header(prof, ROOT / "halalium/sdk/Offsets_generated.h")
    emit_melodium_bridge(ROOT / "internal-main/internal-main/sdk/OffsetsBridge.h")

    # Also refresh curated header TypeInfo block if we have values
    curated = ROOT / "melodium/sdk/Offsets_0.39.2.h"
    if curated.exists() and prof.typeinfo:
        text = curated.read_text(encoding="utf-8")
        for key, addr in prof.typeinfo.items():
            text = re.sub(
                rf"(constexpr uintptr_t {re.escape(key)}\s*=\s*)0x[0-9A-Fa-f]+",
                rf"\g<1>0x{addr:X}",
                text,
            )
        curated.write_text(text, encoding="utf-8")
        (ROOT / "halalium/sdk/Offsets_0.39.2.h").write_text(text, encoding="utf-8")

    print(f"[apply] wrote {gen}")
    print(f"[apply] wrote melodium/sdk/Offsets_generated.h")
    print(f"[apply] wrote OffsetsBridge.h")


def cmd_profile(args: argparse.Namespace) -> int:
    so = Path(args.so)
    script = Path(args.script) if args.script else None
    dump = Path(args.dump) if args.dump else None
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    prof = build_profile(so, script, dump)
    # serialize ints as hex strings for readability
    blob = asdict(prof)
    blob["typeinfo"] = {k: hex(v) for k, v in prof.typeinfo.items()}
    blob["fields"] = {k: hex(v) for k, v in prof.fields.items()}
    (out / "profile.json").write_text(json.dumps(blob, indent=2), encoding="utf-8")
    emit_offsets_header(prof, out / "Offsets_generated.h")
    (out / "features.txt").write_text("\n".join(prof.features) + "\n", encoding="utf-8")
    (out / "hooks.json").write_text(json.dumps(prof.hooks, indent=2), encoding="utf-8")

    print(f"[profile] version={prof.version_string!r} build_id={prof.build_id}")
    print(f"[profile] features={len(prof.features)} typeinfo={len(prof.typeinfo)}")
    print(f"[profile] wrote {out}/profile.json")
    if args.apply:
        apply_profile(out / "profile.json")
    return 0


def cmd_apply(args: argparse.Namespace) -> int:
    apply_profile(Path(args.profile))
    return 0


def cmd_diff(args: argparse.Namespace) -> int:
    old = build_profile(Path(args.old), None, None)
    new = build_profile(Path(args.new), None, None)
    report = {
        "old_version": old.version_string,
        "new_version": new.version_string,
        "old_build_id": old.build_id,
        "new_build_id": new.build_id,
        "features_added": sorted(set(new.features) - set(old.features)),
        "features_removed": sorted(set(old.features) - set(new.features)),
        "strings_lost": [k for k, v in new.strings.items() if not v.get("present") and old.strings.get(k, {}).get("present")],
        "egl_xrefs_old": old.hooks.get("render", {}).get("install_xrefs", []),
        "egl_xrefs_new": new.hooks.get("render", {}).get("install_xrefs", []),
    }
    print(json.dumps(report, indent=2))
    if args.out:
        Path(args.out).write_text(json.dumps(report, indent=2), encoding="utf-8")
    return 0


def cmd_emu_check(args: argparse.Namespace) -> int:
    """Validate Melodium source still matches Halalium emulator contract."""
    main_cpp = (ROOT / "internal-main/internal-main/main.cpp").read_text(encoding="utf-8", errors="replace")
    gui_cpp = (ROOT / "internal-main/internal-main/src/menu/gui.cpp").read_text(encoding="utf-8", errors="replace")
    ok = True
    checks = [
        ("eglSwapBuffers hook present", "hook_egl_swap_buffers" in main_cpp and "eglSwapBuffers" in main_cpp),
        ("PresentFrame not used as primary init", "init_render_hook" in main_cpp and "7B5AD10" not in main_cpp.split("void init_render_hook")[1][:800]),
        ("watermark click open", "##wm_click" in gui_cpp or "wm_click" in gui_cpp),
        ("Lemming watermark", "Lemming" in gui_cpp or "lemminghack" in gui_cpp),
        ("Offsets bridge exists", (ROOT / "internal-main/internal-main/sdk/OffsetsBridge.h").exists()
         or (ROOT / "melodium/sdk/Offsets_0.39.2.h").exists()),
    ]
    for name, passed in checks:
        mark = "OK" if passed else "FAIL"
        print(f"[{mark}] {name}")
        ok = ok and passed
    return 0 if ok else 1


def main(argv: Optional[List[str]] = None) -> int:
    ap = argparse.ArgumentParser(description="Halalium emulator / Melodium update toolkit")
    sp = ap.add_subparsers(dest="cmd", required=True)

    p = sp.add_parser("profile", help="Analyze Halalium SO (+ dumps) into a profile")
    p.add_argument("--so", required=True)
    p.add_argument("--script", default="")
    p.add_argument("--dump", default="")
    p.add_argument("--out", default=str(ROOT / "tools/halalium_emu/out"))
    p.add_argument("--apply", action="store_true", help="also apply into Melodium tree")
    p.set_defaults(func=cmd_profile)

    a = sp.add_parser("apply", help="Apply profile.json into Melodium/halalium sdk headers")
    a.add_argument("--profile", required=True)
    a.set_defaults(func=cmd_apply)

    d = sp.add_parser("diff", help="Diff two Halalium SOs (feature/string surface)")
    d.add_argument("--old", required=True)
    d.add_argument("--new", required=True)
    d.add_argument("--out", default="")
    d.set_defaults(func=cmd_diff)

    c = sp.add_parser("emu-check", help="Check Melodium still matches Halalium contract")
    c.set_defaults(func=cmd_emu_check)

    args = ap.parse_args(argv)
    # normalize optional paths
    if getattr(args, "script", None) == "":
        args.script = None
    if getattr(args, "dump", None) == "":
        args.dump = None
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
