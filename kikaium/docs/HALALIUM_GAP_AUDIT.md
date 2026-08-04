# Halalium ↔ Kikaium gap audit (re-decompile 2026-08-04 #2)

Re-ran `tools/halalium_emu/decompile_funcs.py` including new **SkinChanger @0x1d9e00**.

## DONE

| Item | Notes |
|---|---|
| Menu allowlist | Matches `menu_body` ADRP strings |
| Update / LateUpdate RVA | `0x8E7C40C` / `0x8E7CF50` |
| Photon isLocal `@0x30` | `static_assert` |
| Visible `@0xd8` / Through Walls | `set_visible` only when Through Walls |
| Silent Aim + Bone + Auto Fire + Auto Wall + Fov Check | raycast + circle overlay |
| No spread / Fire Rate / Wallshot / Inf Ammo | `exploits.cpp` |
| AA Pitch / Spin / Reverse Spin | `antiaim.cpp` |
| ESP Box/Corner/Distance/HP/Bone | `esp.cpp` |
| World + Solid + Apply | `world.cpp` |
| Chams / Local / TPS / Watermark | UI + features |
| **Skin Changer** | `skins.cpp` from SkinChanger RE: create`0x8E8FE50` / mid`0x8E852D4` / equip`0x8E7F7F4` |

## STILL OFF (intentional)

| Item | Why |
|---|---|
| InputConsumer hook | Stability |
| getrr Bypass | `use_getrr_bypass=false` |
| Secondary/tertiary egl hooks | Primary Update/LateUpdate sufficient |

## BUGS FIXED THIS PASS

1. Skin Changer was UI-only — now calls Halalium RVAs from Update path
2. Through Walls no longer conflated with bare `set_visible` for every ESP tick (ESP still sets visible byte for occlusion)
3. Added SkinChanger to 2-pass decompile index

## LateUpdate `InstantiateViaServer` note

`@0x1d7ff0` path uses photon props `untouchable` / `untouchableDuration` / `spawn_time` then RPC — **not** the SkinChanger entry. Real skins = `@0x1d9e00` called from Update `@0x1d7dc0`.
