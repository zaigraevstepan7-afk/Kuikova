# Offset audit — privet (`1.zip`) vs Melodium/Halalium 0.39.2

**Offsets were NOT changed.** This is a read-only check against known 0.39.2 sources
(AcademicDLC / hntr111 / Melodium `Offsets_0.39.2.h`).

## Values — match

| Symbol | privet | ref 0.39.2 | status |
|--------|--------|------------|--------|
| PlayerManager TypeInfo | `180740496` (`0xAC5E190`) | same | OK |
| static_fields | `0x90` | `0x90` | OK |
| static instance slot | `0x10` | `0x10` | OK |
| PM local | `0x70` | `0x70` | OK |
| PM list | `0x28` | `0x28` | OK |
| list buffer / entry / stride | `0x18` / `0x30` / `0x18` | same | OK |
| team | `0x79` | `0x79` | OK |
| movement | `0x98` | `0x98` | OK |
| photon | `0x160` | `0x160` | OK |
| main camera | `0xE8` | `0xE8` | OK |
| cam matrix | `0xF0` | `0xF0` | OK |
| photon name / props | `0x20` / `0x38` | same | OK |
| char view | `0x48` | `0x48` | OK |
| occlusion | `0xB8` | `0xB8` | OK |
| weaponry / current | `0x88` / `0xA0` | same | OK |
| char visible | `0xD8` | `0xD8` | OK |
| CharacterLOD / mesh | `0x128` / `0x30` | same | OK |
| movement transform data / pos | `0xB0` / `0x44` | same | OK |
| il2cpp API RVAs (`il2cpp.hpp`) | api_0.39.2 | same set | OK |

## Suspect / notes (values left as-is)

| Symbol | privet | note |
|--------|--------|------|
| `OFF_PLAYER_HEALTH` `0x7C` | Melodium layout: `0x7C` = `m_fLocalTime`, not HP. Runtime already falls back to photon props `"health"` — primary float read is dubious. |
| `OFF_PLAYER_ARMS_LOD` `0xC8` | Melodium primary arms LOD is `0x120`; `0xC8` is alt path. OK if unused for ESP. |
| `OFF_CAMERA_TRANSFORM` `0x100` | Unused. Live path is `cam+0x20 → +0x10 → matrix@0xF0` (correct). |

## Architecture bugs found (fixed in code, offsets untouched)

1. **`g_base` resolved `libunity.so`**, but TypeInfo `0xAC5E190` and all `il2cpp_*` API RVAs are **`libil2cpp.so`**.
2. **`segment_resolve_rva` scanned `libunity.so` maps** → API binds pointed at wrong module.
3. Monolithic `main.cpp` (~1100 lines) mixed mem / modules / game / esp / tps / hooks.

## Features in this tree

ESP (box / health / distance / skeleton) + third person. **No chams** in original `1.zip`.
