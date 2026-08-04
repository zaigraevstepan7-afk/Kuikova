# Kikaium — full Halalium reverse → buildable Melodium-style project

## What this is

**Kikaium** is our cheat built like Melodium, driven by **Halalium RE** (not a binary paste).

| Path | Role |
|------|------|
| `halalium/bin/libhalalium.so` | Closed reference binary (RE target) |
| `kikaium/project/` | **Buildable source** (CMake → `libkikaium.so`) |
| `kikaium/bin/libkikaium.so` | Output of `project/` build |
| `kikaium/sdk/` | Halalium offsets (0.39.2) |
| `kikaium/docs/` | RE profile / hooks / this map |
| `tools/halalium_emu/` | Auto-RE when a new Halalium drops |

## Architecture (from Halalium RE)

```text
AndKittyInjector --memfd --libs libkikaium.so
        │
        ▼
ctor / JNI_OnLoad → process-once → entry()
        │
        ├─ dlsym(libEGL, eglSwapBuffers) + A64/GOT hook  (Halalium: DobbyHook)
        │     └─ ImGui + ##wm_click + c_esp/c_visual overlays (when g_sdk_ready)
        │
        └─ resolve libil2cpp/libunity base
              └─ update::init: ::init() + VMT Game/Player Update
                    + hit/gun VMT + optional ray icall
```

### Confirmed Halalium facts (0.39.2)

| Item | Value |
|------|--------|
| BuildID | `12532fca99debbaa836dbbea6e5cceec95f5bbbb` |
| Render | `dlsym(eglSwapBuffers)` + Dobby @ install `0x1d84e8` |
| Menu open | `##wm_click` on watermark (not Insert) |
| Brand (upstream) | Lemming / t.me/lemminghack, 0.39.2 |
| Threads | `Halalium_Hooks`, `Halalium_Bypass` |
| Maps | `/proc/self/maps` for module base |
| Memory | in-process LDR/STR (no `process_vm_*`) |
| `Il2CppClass.static_fields` | **0x90** |
| Player +0x160 | PhotonPlayer* |
| Player +0x88 | WeaponryController* |
| Weaponry +0xA0 | WeaponController* |
| PlayerManager TypeInfo | `0xAC5E190` |
| Update RVA (feng) | `0x8E7C40C` / LateUpdate `0x8E7CF50` |
| getrr / OnStart | `0x8B9579C` — needs Dobby reloc; **off** in Kikaium |
| padla | chams shaders only (UnityFS) |

### Features present in Halalium strings

Silent Aim, Anti Aim, Enable Esp, Auto Wall, Through Walls, Wallshot, Skin Changer,
Chams, Inf Ammo, No spread, Auto Fire, Third Person, Fov Check, Health Bar, Bone/Box, …

Kikaium ports Melodium feature code on this map.

## Build (phone arm64)

```bash
# NDK must be installed (this env: android-ndk-r27c)
cd kikaium/project
cmake --preset release-phone
cmake --build build/release-phone --parallel
# → kikaium/bin/libkikaium.so
```

Or:

```bash
bash kikaium/build.sh          # builds once
bash kikaium/build.sh --verify # builds 3 times and checks SO
```

## Refresh offsets from a new Halalium

```bash
bash tools/halalium_emu/update.sh \
  halalium/bin/libhalalium.so \
  okak/okaakka/script.json \
  "okak/okaakka/dump (1).cs"
# then rebuild kikaium/project
```

## Inject

Same as Halalium:

```text
AndKittyInjector --package com.axlebolt.standoff2 --libs libkikaium.so --memfd --delay 2000000
```

Tap watermark **Kikaium** to open menu.
