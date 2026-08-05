# Halalium vs NOT — offset source table (Kikaium)

Rule: **runtime hardcoded RVAs/fields must come from Halalium RE only**
(`halalium_profile.json`, `docs/decompile/*`, `Offsets_generated.h`).
Everything else is MethodInfo/`dlsym` resolve or disabled.

---

## ✅ FROM HALALIUM (used)

### TypeInfo (libil2cpp)

| Symbol | Value | Source |
|---|---|---|
| PlayerManager | `0xAC5E190` | profile |
| GameController | `0xAC58BB0` | profile |
| PhotonNetwork | `0xAC5DE18` | profile |
| BombManager | `0xAC4FAC0` | profile |
| InventoryManager | `0xAC5C018` | profile |
| PlayerControls | `0xAC5E0E0` | profile |
| PlayerController | `0xAC5E0D8` | profile |
| WeaponController | `0xAC61A18` | profile |
| WeaponManager | `0xAC61A78` | profile |
| GameManager | `0xAC58C00` | profile |
| TouchController | `0xAC60B48` | profile |
| AntiCheatManager | `0xAC4DA30` | profile |
| GunController | `0xAC59040` | profile |
| PlayerMainCamera | `0xAC5E188` | profile |

### Fields

| Field | Off | Source |
|---|---|---|
| Il2Cpp.klass_static_fields | `0x90` | profile |
| PlayerManager.players_list / local / alt | `0x28` / `0x70` / `0x68` | profile |
| Player.photon_player | `0x160` | profile + Update LDR |
| Player.weaponry | `0x88` | profile |
| Weaponry.weapon | `0xA0` | profile |
| Player.occlusion | `0xB8` | profile |
| Player.main_camera | `0xE8` | profile |
| Player.team | `0x79` | profile |
| Player.aim | `0x80` | profile |
| Player.hit | `0xA8` | profile |
| Player.movement | `0x98` | profile |
| Player.character_view | `0x48` | profile |
| Player.photon_view | `0x150` | profile |
| Player.character_visible | `0xD8` | Update `strb` |
| Photon.isLocal | `0x30` | Update `ldrb` |
| GameController.player_controls / local | `0x2B0` / `0x2C0` | profile |
| Camera.matrix | `0xF0` | profile |
| ESP camera chain | `Player+0xE8 → +0x28 → +0x30 → @0xF0` | Halalium Update FOV nest (only path) |

### Method RVAs (libunity)

| Method | RVA | Source |
|---|---|---|
| PlayerController.Update | `0x8E7C40C` | egl_install |
| PlayerController.LateUpdate | `0x8E7CF50` | Update+`0xB44` |
| Secondary / Tertiary / TertiaryAlt | `0x8E0085C` / `0x79FE5E0` / `0x147E970` | egl_install |
| ExtraA / ExtraB | `0x8D663EC` / `0x8D2B2B0` | egl_install |
| getrr OnStart | `0x8B9579C` | profile / Bypass |
| Skin Create/Mid/Equip | `0x8E8FE50` / `0x8E852D4` / `0x8E7F7F4` | SkinChanger |

### UnityMethod RVAs (embedded in Halalium callbacks)

| Method | RVA | Source |
|---|---|---|
| set_fieldOfView | `0x5FB7BC0` | Update_Halalium_Hooks |
| GameObject.SetActive | `0x6009294` | secondary_hook_cb |
| Component.get_gameObject | `0x5FFAEFC` | secondary_hook_cb |
| Update aux A/B | `0x8C69CB0` / `0x8E8AE24` | Update (stored, unused) |
| LateUpdate A/B/C | `0x8C7BE04` / `0x5CAAFC4` / `0x5DEADA4` | LateUpdate (stored, unused) |

---

## ❌ NOT FROM HALALIUM (removed / not hardcoded)

Former Melodium/`globals.hpp` oxorany RVAs — **deleted from `class offsets`**.
Features that need them use **MethodInfo resolve** (`globals::init` → `c_fn` / `c_methods`) or are **disabled**.

| Former member | Old RVA | Status now |
|---|---|---|
| get_touch / get_count | `0x684EDAC` / `0x684E370` | MethodInfo `Input` |
| get_width / get_heigth | `0x5FB3224` / `0x5FB5478` | unused (EGL size) |
| get/set_position, forward, up, euler, rotation | `0x6005…` | MethodInfo `Transform` |
| set_tps / set_fps / set_visible / set_view_mode | `0x8E7E…` | MethodInfo name scan / field `0xD8` |
| Shader/Material/Renderer/Type find* | `0x6A…` / `0x5C…` | MethodInfo |
| fog / RenderSettings_* | `0x6A8…` | **removed** (Melodium) |
| linecast / sphere_cast / get_velocity | `0x7A…` | MethodInfo |
| ray icall slot | `0x84DB9F0` | **removed** |
| icall resolve | `0x5CAAE90` | `dlsym(il2cpp_resolve_icall)` |
| get_main / set_flags / bagcolor / matrices | `0x5FA…` | MethodInfo / Halalium `set_fov` |
| get_transform | `0x5FF113C` | MethodInfo |
| create_string | `0x5DE99CC` | MethodInfo |
| il2cpp_domain_get / assembly_* / class_from_name / object_new | `0x5C86…` / `0x9D5B510` | **`dlsym` exports** (not Halalium SO) |

### Extra field oxoranys removed from features

| Site | Off | Status |
|---|---|---|
| esp Camera | `Player+0xE8 → +0x28 → +0x30 → matrix@0xF0` | Halalium only |
| esp ammo capacity `+0x130` | community | **removed** |
| visual HUD `+0xB8/+0x30/+0x50` | Melodium | **removed** (keep Halalium `set_fov`) |
| antiaim TranslationData `+0xE0` | community | **disabled** |
| chams weapon LOD `+0x48` | Melodium | **disabled** |

### Still struct layout (not RVA) from dump + Halalium Skin comment

Weapon `parameters@0xA8`, id`@0x18`, skin cache`@0xE0` — from SkinChanger RE notes / dump structs, not Melodium method RVAs.

---

## Bootstrap note

`il2cpp_*` APIs are **not** embedded in Halalium SO. Kikaium uses **`dlsym("libil2cpp.so")`** so we do not hardcode community Api RVAs.

---

## Read / Write model (Halalium → Kikaium)

Halalium after inject is **in-process**: plain `LDR`/`LDRB`/`STR`/`STRB` + `cbz` null checks.
`/proc/self/maps` only for **module base** (`libunity.so`) — **not** `process_vm_*`.

| Primitive | Halalium | Kikaium |
|---|---|---|
| Module base | maps → `libunity` | `unity_base` + `base`(il2cpp) |
| TypeInfo | `*(il2cpp + RVA)` | `hmem::typeinfo(base, RVA)` |
| Singleton | klass`+0x90` → statics → inst | `hmem::typeinfo_instance` |
| Field read | `ldr [obj,#off]` | `hmem::field` / `safe_type` / struct |
| Field write | `strb`/`str` | `hmem::set_field` / `safe_t::set` |
| Safe int/float | helpers `@0x1d8c30`/`@0x1d8c54` | `safe_t` XOR / byte-swap |
| Chains | Update nests | `includes/halalium_chains.h` |

### Feature write sites (Update RE)

| Feature | Halalium write | Kikaium |
|---|---|---|
| Through Walls | `strb 1 → player+0xD8` | `hmem::set_field` |
| Inf Ammo | SafeInt `weapon+0x120/0x128 = 0x45` | `m_iCapacitySafe`/`m_iAmmoSafe` |
| Fire Rate | SafeFloat `weapon+0x108` | `m_fTimeFiredSafe` + interval |
| Wallshot | Safe `params+0x2DC/0x264/0x258` | same + named dump fields |
| No spread | ExtraB clear + accuracy | accuracy params zeroed |
| Skins | Create/Mid/Equip RVAs | `unity_base + Skin_*` |
| FOV | `blr unity+0x5FB7BC0` | `c_fn->set_fov` |
