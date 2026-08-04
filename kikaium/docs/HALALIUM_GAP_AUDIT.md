# Halalium ↔ Kikaium parity audit (100% pass)

Re-decompiled with `tools/halalium_emu/decompile_funcs.py` (15 funcs including SkinChanger).
Source: `halalium/bin/libhalalium.so` BuildID `12532fca…bbbb`.

## Offset parity (2026-08-04)

Automated compare: `halalium_profile.json` + decompile JSONs + `libhalalium.so` bytes
vs `Offsets_generated.h` / `Offsets_0.39.2.h` / `c_playercontroller.h` / `c_photon_player.h` / `globals.hpp`.

### Method RVAs (libunity base) — 11/11 OK

| Symbol | Halalium | Kikaium | Source |
|---|---|---|---|
| PlayerController.Update | `0x8E7C40C` | same | egl_install |
| PlayerController.LateUpdate | `0x8E7CF50` | same | Update+`0xB44` |
| Secondary | `0x8E0085C` | same | egl_install |
| Tertiary | `0x79FE5E0` | same | egl_install |
| TertiaryAlt | `0x147E970` | same | egl_install |
| ExtraA | `0x8D663EC` | same | egl_install |
| ExtraB | `0x8D2B2B0` | same | egl_install (movz `#0xb2b0` + movk `#0x8d2`) |
| getrr OnStart | `0x8B9579C` | same | profile / FULL_RE |
| Skin CreateOrGet / Mid / Equip | `0x8E8FE50` / `0x8E852D4` / `0x8E7F7F4` | same | SkinChanger |

`Gun_ExecuteCommands` / `Hit_StrictHit` = `0x0` — name VMT only (intentional).

### Fields — 18/18 profile OK + struct

| Field | Off | Status |
|---|---|---|
| Photon.isLocal | `0x30` | OK (`static_assert`) |
| Player.team | `0x79` | OK |
| Player.weaponry | `0x88` | OK |
| Player.movement | `0x98` | OK |
| Weaponry.weapon | `0xA0` | OK |
| Player.hit | `0xA8` | OK |
| Player.occlusion | `0xB8` | OK |
| Player.visible | `0xD8` | OK (struct + `Offsets::Player::character_visible`) |
| Player.main_camera | `0xE8` | OK |
| Player.photon | `0x160` | OK |
| PM list/local/alt | `0x28`/`0x70`/`0x68` | OK |
| GC controls/local | `0x2B0`/`0x2C0` | OK |
| Camera.matrix | `0xF0` | OK |
| Il2Cpp.static_fields | `0x90` | OK |

### TypeInfo (libil2cpp) — 14/14 OK

Including `GunController` `0xAC59040`, `PlayerMainCamera` `0xAC5E188`.

### API / methods in `globals.hpp` (sample)

`il2cpp_domain_get` `0x5C86D54`, `il2cpp_class_from_name` `0x9D5B510`, `set_fov` `0x5FB7BC0` (also in Update RE), `SetActive` `0x6009294`, `get_game_object` `0x5FFAEFC` — match Api/Halalium secondary LDR.

### Not Halalium-critical / unverified

| Item | Note |
|---|---|
| `ray` icall slot `0x84DB9F0` | TODO live verify on device |
| Fog / Melodium-only method RVAs | Present in globals; unused by Halalium allowlist UI |
| Module base | Method RVAs → `unity_base`; TypeInfo/API → `base` (il2cpp) |

## Menu — allowlist / child IDs

| Child | Contents (menu_body ADRP order) |
|---|---|
| `##vis_left` | Enable Esp, Box, Box Type/Color, Corner Size, Health Bar, Distance/Color |
| `##vis_right_top` | Chams, Enemy Chams/Color, Through Walls |
| `##vis_right_bottom` | World/Color, Solid/Apply, Spin, Spin Speed, Reverse Spin |
| `##rage_left` | Silent Aim, Bone, Fov Check/Fov/Color, Auto Fire/Wall, No spread |
| `##rage_right_top` | Anti Aim, Local Chams/Color |
| `##rage_right_bottom` | Anti Aim Pitch, Enable Anti Aim first |
| `##misc_left` | Third Person |
| `##misc_right_top` | Inf Ammo, Fire Rate, Wallshot |
| `##misc_right_bottom` | present (empty in SO) |
| `##settings_left` | Accent Color |
| `##settings_watermark` | Watermark |
| `##skins_panel` / `##weapons_list` / `##skins_list` | Skin Changer |

Layout: right column stacks top+bottom under one parent (not three SameLine siblings).

## Hooks — install flags (lobby-safe)

| Halalium | RVA | Kikaium |
|---|---|---|
| Update | `0x8E7C40C` | tracked a64 ON (`unity_base`) |
| LateUpdate | `0x8E7CF50` | tracked a64 ON |
| Secondary / Tertiary / ExtraA/B | see above | **OFF** (`use_secondary_hooks`) |
| eglSwapBuffers | dlsym | ON |
| InputConsumer | libinput | **ON** (`use_input_consume` — Halalium AInputEvent path) |
| getrr Bypass | `0x8B9579C` | **OFF** (`use_getrr_bypass`) |
| GameController VMT | — | ON (hardened) |

## Features

Silent Aim, Bone (rage), Fov Check, Auto Fire/Wall, No spread, Inf Ammo, Fire Rate, Wallshot, AA+Pitch, Spin+Reverse, ESP Box/Corner/Distance/HP/Skeleton-with-Esp, Chams, Through Walls, World+Apply, Skin Changer, Third Person, Watermark.

## Melodium

Still absent from UI (fog/hitmarkers/name ESP/duck/DT/… ).
