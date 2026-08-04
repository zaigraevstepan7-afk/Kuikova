# Halalium ↔ Kikaium parity audit (100% pass)

Re-decompiled with `tools/halalium_emu/decompile_funcs.py` (15 funcs including SkinChanger).
Source: `halalium/bin/libhalalium.so`.

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

## Hooks — Halalium egl_install

| Halalium | RVA | Kikaium |
|---|---|---|
| Update | `0x8E7C40C` | tracked a64 ON |
| LateUpdate | `0x8E7CF50` | tracked a64 ON |
| Secondary | `0x8E0085C` | tracked a64 ON (passthrough) |
| Tertiary | `0x79FE5E0` / alt `0x147E970` | tracked a64 ON (float ABI passthrough; silent via ray) |
| ExtraA | `0x8D663EC` | tracked a64 ON (passthrough) |
| ExtraB | `0x8D2B2B0` | tracked a64 ON (+ No spread `*a1=0`) |
| eglSwapBuffers | dlsym | ON |
| InputConsumer | libinput | symbol probe; touch via ImGui |
| getrr Bypass | `0x8B9579C` | **ON** via `hhooks::install_getrr_bypass` |

## Features

Silent Aim, Bone (rage), Fov Check, Auto Fire/Wall, No spread (gun + ExtraB), Inf Ammo, Fire Rate, Wallshot, AA+Pitch, Spin+Reverse, ESP Box/Corner/Distance/HP/Skeleton-with-Esp, Chams, Through Walls, World+Apply, Skin Changer (create/mid/equip), Third Person, Watermark.

## Melodium

Still absent from UI (fog/hitmarkers/name ESP/duck/DT/… ).
