# Melodium → Standoff 2 **0.39.2** offset update

Sources: okaakka `script.json` / dump, feng `0392_dump_arm64_v8a`, `api_0.39.2.txt`, Halalium RE, community AcademicDLC offsets.

## Updated

| Area | Change |
|------|--------|
| `globals.hpp` | All TypeInfo + Unity/il2cpp method RVAs from 0.36.x → 0.39.2 |
| `globals.cpp` | `Il2CppClass.static_fields` **0xB8 → 0x90**; `type_info_instance` uses `field` |
| `c_playercontroller.h` | Full PlayerController layout (Photon **0x160**, team **0x79**, …) |
| `c_weapon_controller.h` | Weapon/Gun layouts (params **0xA8** / **0x168**, LOD **0x88**) |
| `update.cpp` | GameController.PlayerControls **0x298 → 0x2B0** |
| `structs.h` | input filter Action **0xB8 → 0xC0** |
| `visual.cpp` | HUDView **0xB0 → 0xB8** |
| `esp.cpp` | weapon params **0xA0 → 0xA8**; view matrix **0x100 → 0xF0** |
| `world.cpp` | Fog via `UnityEngine.RenderSettings` APIs |

## Still TODO on-device

- `present_frame` slot `0x7B5AD10` (menu GLES hook) — confirm against live `libil2cpp`/`libunity`
- `ray` icall table slot `0x84DB9F0` — PhysicsScene raycast hook
- Obfuscated `set_tps` / `set_fps` / `set_visible` RVAs are best-effort from dump adjacency; swap if third-person/visibility misbehaves
- GunController VMT hook name `FEEBGAGHGGCGACA` / vtable index may have changed with obfuscation
