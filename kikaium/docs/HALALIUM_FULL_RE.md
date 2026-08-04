# Halalium full reverse map → Kikaium

Generated/maintained for Kikaium builds. Upstream binary: `halalium/bin/libhalalium.so`.

Refresh:
```bash
bash tools/halalium_emu/update.sh
bash kikaium/build.sh --verify
```

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
| Menu open | InvisibleButton `##wm_click` → toggle open |
| Watermark | `##watermark`, brand `Lemming` |
| ImGui | Dear ImGui + `imgui_impl_opengl3` |

**Kikaium:** same egl symbol hook; watermark brand `Kikaium`; click id `##wm_click`; copper UI; ESP/hit overlays on swap after `g_sdk_ready`.

## 3. Threads / bypass

| String | Role |
|--------|------|
| `Halalium_Hooks` | feature / hook init |
| `Halalium_Bypass` | getrr / OnStart path |
| getrr | `ECGCHECCBBBAEBB$$OnStart` @ RVA `0x8B9579C` |

**Kikaium:** getrr **disabled** (`Hook::use_getrr_bypass = false`).

## 4. Memory model

In-process LDR/STR after inject (no `process_vm_*`). `/proc/self/maps` for bases.

**Kikaium:** `includes/halalium_mem.h` + `egl/memory.cpp`.

## 5. Il2Cpp / TypeInfo (0.39.2)

```text
klass = *(libil2cpp + TypeInfo_RVA)
statics = *(klass + 0x90)
instance = *(statics + 0x10)   // common singleton slot
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
| PlayerMainCamera | 0xAC5E188 |
| WeaponController | 0xAC61A18 |
| WeaponManager | 0xAC61A78 |
| GunController | 0xAC59040 |
| GameManager | 0xAC58C00 |
| TouchController | 0xAC60B48 |
| AntiCheatManager | 0xAC4DA30 |

### Fields (dump.cs brace-parse + Halalium LDR)

| Path | Off |
|------|-----|
| PlayerManager.players_list | 0x28 |
| PlayerManager.alt_player | 0x68 |
| PlayerManager.local_player | 0x70 |
| Player.character_view | 0x48 |
| Player.team | 0x79 |
| Player.aim_controller | 0x80 |
| Player.weaponry | 0x88 |
| Player.movement | 0x98 |
| Player.hit_controller | 0xA8 |
| Player.occlusion | 0xB8 |
| Player.main_camera | 0xE8 |
| Player.photon_view | 0x150 |
| Player.photon_player | 0x160 |
| Weaponry.weapon_controller | 0xA0 |
| GameController.player_controls | 0x2B0 |
| GameController.local_player | 0x2C0 |
| Camera.matrix | 0xF0 |

### Methods

| Name | RVA | Notes |
|------|-----|-------|
| PlayerController.Update | 0x8E7C40C | Halalium A64 target; Kikaium uses VMT `"Update"` |
| AntiCheat OnStart (getrr) | 0x8B9579C | bypass off |

## 6. Feature surface (Halalium UI strings → Kikaium)

23 labels recovered from SO (Enable Esp, Silent Aim, Auto Wall, Chams, …).  
Kikaium menu: Melodium-style tabs (`esp`, `silent`, `chams`, `anti aims`, …) via oxorany.

## 7. Kikaium product contract

1. `eglSwapBuffers` draw path (not PresentFrame)
2. Menu open via `##wm_click`
3. Offsets from `sdk/generated/Offsets_generated.h`
4. VMT hooks: GameController/PlayerController Update + LateUpdate + hit/gun
5. Overlay: `c_esp->render()` + visual hits when `g_sdk_ready`
6. Process-once inject guard (`kikaium.once`)
