# Halalium → Kikaium reconstruction map (deep RE)

Binary: `halalium/bin/libhalalium.so` BuildID `12532fca…` / `t.me/lemminghack, 0.39.2`

## Confirmed install path (`egl_install` @ `0x1d84cc`)

1. `dlsym(libEGL.so, eglSwapBuffers)` → `DobbyHook(cb=0x1d76f0, &orig@0x279578)`
2. `dlsym(libinput, InputConsumer::consume)` → `DobbyHook(cb=0x1d760c, &orig@0x279580)`
3. Game base from slot `@0x2794f0` (libunity), then DobbyHook:

| RVA | Callback | Role |
|-----|----------|------|
| `0x8E7C40C` | `0x1d7a0c` | PlayerController.Update (Halalium_Hooks body) |
| `0x8E0085C` | `0x1d81fc` | secondary |
| `0x79FE5E0` (alt `0x147E970`) | `0x1d8404` | tertiary |
| `0x8E7CF50` (=Update+0xB44) | `0x1d7ec4` | LateUpdate |
| `0x8D663EC` | `0x1d82a0` | extra A |
| `0x8D2B2B0` | `0x1d83cc` | extra B |

## Update callback (`0x1d7a10` / Halalium_Hooks)

- `x0` = PlayerController*
- `ldr x8,[x19,#0x160]` PhotonPlayer*
- local check `ldrb [photon,#0x30]`
- stores local `@0x2795b0`
- forces visibility / feature tick (ESP/aim/chams gated by menu flags)
- LDR confirmations: player+0x160, +0x88 weaponry path, team `@0x79`, visible `@0xd8`

## Bypass (`0x1d90b8`)

1. log `Halalium_Bypass`
2. iterate tracked hooks → `DobbyDestroy`
3. `blr` real OnStart (`orig@0x2796f0` / RVA `0x8B9579C` getrr)
4. reinstall via `DobbyHook`
5. log `bypas hok result %d`

## Menu

- Always draw `##watermark` (Lemming / version)
- `##wm_click` → toggle byte `@0x279064`
- Full menu gated on open flag
- Tabs: Rage / Visuals / Misc / Settings / Skins (WildRage IA)
- Feature labels: Enable Esp, Silent Aim, Auto Wall, Through Walls, Wallshot, No spread, Inf Ammo, Chams, Skin Changer, Anti Aim, Spin, Fov Check, …

## Kikaium reconstruction status

| Halalium piece | Kikaium |
|----------------|---------|
| eglSwapBuffers draw | ✅ |
| ##wm_click open (closed default) | ✅ |
| Absolute RVA Update/LateUpdate | ✅ tracked a64 (Dobby-equiv) + VMT fallback |
| InputConsumer | ✅ (AInputEvent_getType/getX/Y like Halalium) |
| getrr Bypass thread | scaffolding only (off) |
| Feature UI labels | ✅ Halalium names |
| Melodium widget quality | ✅ left rail + copper |
| padla fresnel/shador | ❌ (needs AssetBundle load) |
| Skin InstantiateViaServer | UI wired, runtime path TBD |
| ESP/visual overlays on swap | ✅ |

Refresh: `bash tools/halalium_emu/update.sh && bash kikaium/build.sh --verify`
