# Halalium full reverse map → Kikaium

Generated/maintained for Kikaium builds. Upstream binary: `halalium/bin/libhalalium.so`.

## 1. Binary identity

| Field | Value |
|-------|--------|
| File | libhalalium.so |
| Arch | aarch64 ELF shared |
| Size | ~2.58 MB stripped |
| BuildID | `12532fca99debbaa836dbbea6e5cceec95f5bbbb` |
| Version string | `t.me/lemminghack, 0.39.2` |
| Inject | AndKitty `--memfd` into `com.axlebolt.standoff2` |

## 2. Render / UI (static RE)

| Piece | Evidence |
|-------|----------|
| Hook strategy | `dlsym("libEGL.so","eglSwapBuffers")` + `DobbyHook` |
| Install PC | `0x1d84e8` (xref from egl string load) |
| Callback | draws ImGui each frame, then orig swap |
| Menu open | InvisibleButton `##wm_click` → toggle byte `@0x279064` |
| Watermark | `##watermark`, brand `Lemming`, line version string |
| Settings WM | `##settings_watermark` |
| ImGui | Dear ImGui + `imgui_impl_opengl3` |
| Input | also hooks `android::InputConsumer::consume` (libinput) |

**Kikaium mapping:** same egl symbol hook (A64 inline / GOT), watermark click open, copper UI (not Lemming chrome).

## 3. Threads / bypass labels

| String | Role |
|--------|------|
| `Halalium_Hooks` | feature / hook init logging |
| `Halalium_Bypass` | getrr / OnStart path |
| getrr | obfuscated anti-cheat OnStart @ RVA `0x8B9579C` |
| Bypass pattern | destroy hooks → call orig → reinstall (needs relocating hooker) |

**Kikaium:** getrr **disabled** until Dobby-class reloc exists (`use_getrr_bypass = false`).

## 4. Memory model

Halalium does **not** use `process_vm_readv/writev`. After inject it uses in-process LDR/STR with null checks; `/proc/self/maps` for module bases / soft validity.

**Kikaium:** `includes/halalium_mem.h` + `egl/memory.cpp`.

## 5. Il2Cpp / TypeInfo (0.39.2)

```text
klass = *(libil2cpp + TypeInfo_RVA)
statics = *(klass + 0x90)          // Il2CppClass.static_fields
instance = *(statics + 0x10)       // common singleton slot
```

| TypeInfo | RVA |
|----------|-----|
| PlayerManager | 0xAC5E190 |
| GameController | 0xAC58BB0 |
| PhotonNetwork | 0xAC5DE18 |
| InventoryManager | 0xAC5C018 |
| BombManager | 0xAC4FAC0 |
| PlayerController | 0xAC5E0D8 |
| PlayerControls | 0xAC5E0E0 |
| WeaponController | 0xAC61A18 |
| WeaponManager | 0xAC61A78 |
| GameManager | 0xAC58C00 |
| TouchController | 0xAC60B48 |
| AntiCheatManager | 0xAC4DA30 |

### Fields (Halalium LDR + dump)

| Path | Off |
|------|-----|
| PlayerManager.players_list | 0x28 |
| PlayerManager.local_player | 0x70 |
| Player.photon_player | 0x160 |
| Player.weaponry | 0x88 |
| Player.occlusion | 0xB8 |
| Player.main_camera | 0xE8 |
| Player.team | 0x79 |
| Weaponry.weapon | 0xA0 |
| GameController.PlayerControls | 0x2B0 |

### Method RVAs (feng 0.39.2 / Halalium install immediates)

| Method | RVA |
|--------|-----|
| PlayerController.Update | 0x8E7C40C |
| PlayerController.LateUpdate | 0x8E7CF50 |

**Kikaium default:** VMT name hooks (stable). A64 stolen-byte tramp optional / off.

## 6. Feature surface (strings in SO)

Enable Esp, Silent Aim, Rage, Anti Aim, No spread, Auto Fire, Auto Wall, Through Walls,
Wallshot, Skin Changer, Chams, Local Chams, Spin, Fov Check, Health Bar, Bone, Box,
Third Person, Watermark, Inf Ammo, …

## 7. padla

UnityFS AssetBundle with `fresnel.shader` / `shador.shader` for chams only. Not offsets.

## 8. Why Melodium features died historically

1. Menu drew (egl OK) but TypeInfo/`static_fields` wrong → null managers  
2. Field drift (occlusion, PlayerControls)  
3. Method RVA not refreshed  
4. Double inject / double egl hook → crash after ImGui  

Kikaium project ships correct Halalium offsets + process-once + VMT init.

## 9. Re-run RE

```bash
python3 tools/halalium_emu/halalium_emu.py profile \
  --so halalium/bin/libhalalium.so \
  --script okak/okaakka/script.json \
  --dump "okak/okaakka/dump (1).cs" \
  --out tools/halalium_emu/out --apply
```

Artifacts: `kikaium/docs/halalium_profile.json`, `halalium_hooks.json`, `sdk/Offsets_*`.
