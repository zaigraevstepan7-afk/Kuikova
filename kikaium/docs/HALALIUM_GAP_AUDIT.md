# Halalium ↔ Kikaium gap audit (re-decompile 2026-08-04)

Re-ran `tools/halalium_emu/decompile_funcs.py` (Pass A disasm + Pass B fields/RVAs/ADRP+ADD strings).

## DONE (wired)

| Item | Evidence |
|---|---|
| Menu allowlist labels | `c_user_interface.cpp` matches `menu_body` ADRP strings |
| Update/LateUpdate RVA hooks | `0x8E7C40C` / `0x8E7CF50` via `update::init` |
| Photon isLocal `@0x30` | `static_assert` + Halalium `ldrb [photon,#0x30]` |
| Visible `@0xd8` / Through Walls | `m_bCharacterVisible` + `set_visible` |
| Silent Aim + Bone + Auto Fire | raycast + executecommands |
| Auto Wall | skip `is_bone_visible` when on |
| Fov Check (+ circle) | `globals::in_fov` + overlay circle |
| No spread / Fire Rate / Wallshot / Inf Ammo | `exploits.cpp` |
| AA Pitch / Spin / Reverse Spin | `antiaim.cpp` |
| ESP Box / Corner / Distance / Health / Bone | `esp.cpp` |
| World + Solid + Apply | `world.cpp` (Apply commits) |
| Chams / Local Chams / Third Person / Watermark | UI + features |
| ##wm_click closed-by-default menu | `gui.cpp` |

## MISSING / INCOMPLETE

| Item | Status |
|---|---|
| **Skin Changer runtime** | UI + arming log only. Halalium LateUpdate builds Photon RPC (`InstantiateViaServer` @ string `0x3520c`, helpers `0x1d8858`/`0x1d8f*`). Full box/RPC wire still TBD. |
| InputConsumer hook | Present in Halalium `egl_install`; intentionally OFF in Kikaium |
| getrr Bypass | Scaffolded in `halalium_hooks.h`; `use_getrr_bypass` OFF until proven stable |
| Secondary/tertiary/extra hooks (`0x8E0085C`, `0x79FE5E0`, …) | Not installed (Halalium egl_install list); primary Update/LateUpdate covered |
| ##weapons_list / ##skins_list UI polish | Sliders only (weapon id / skin id) |

## BUGS FIXED THIS PASS

1. Silent Aim required `g.b_esp` — **wrong**; Halalium Silent Aim is independent
2. Fov Check was UI-only — now filters aim + draws circle
3. Anti Aim Pitch applied during Spin-alone — gated to `b_antiaim`
4. Melodium `hit_chams` path could `return` before `old_update` — removed
5. Melodium hitmarker/aspect/scope overlay paths stripped from hot path
6. Fov Check duplicated under Visuals — Halalium only under ##rage_left
7. Photon `isLocal` offsetof asserted `@0x30`

## Melodium (must stay absent)

No UI labels for fog/sky/hitmarkers/name-ammo ESP/duck/DT/jitter/chaos/strafer/aspect/scope/god/OHK. Flags forced false.
