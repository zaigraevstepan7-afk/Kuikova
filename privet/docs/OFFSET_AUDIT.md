# Offset audit — privet (`1.zip`) vs Melodium/Halalium 0.39.2

**Offsets were NOT changed.** Read-only check. Runtime base is **`libunity.so`** as in original `1.zip` (reverted from a libil2cpp experiment).

## Values — match

| Symbol | privet | ref 0.39.2 | status |
|--------|--------|------------|--------|
| PlayerManager TypeInfo | `180740496` (`0xAC5E190`) | same | OK |
| static_fields / slot | `0x90` / `0x10` | same | OK |
| PM local / list | `0x70` / `0x28` | same | OK |
| list layout | `0x18` / `0x30` / `0x18` | same | OK |
| team / movement / photon | `0x79` / `0x98` / `0x160` | same | OK |
| main camera / matrix | `0xE8` / `0xF0` | same | OK |
| char view / LOD / visible | `0x48` / `0x128`+`0x30` / `0xD8` | same | OK |
| il2cpp API RVAs | api_0.39.2 | same | OK |

## Notes (unchanged)

- `OFF_PLAYER_HEALTH 0x7C` — Melodium layout says `m_fLocalTime`; HP also via photon props.
- `OFF_PLAYER_ARMS_LOD 0xC8` — alt path (Melodium primary `0x120`).
- Base module: **`libunity.so`** (original privet behavior).
