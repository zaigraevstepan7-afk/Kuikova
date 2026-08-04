# xxx hide = Halalium / Lemming (`libhalalium.so`)

RE of `libhalalium.so` (t.me/lemminghack, 0.39.2) BuildID `12532fca…bbbb`.

## How Halalium hides

### Inject
```sh
AndKittyInjector --package com.axlebolt.standoff2 --libs libhalalium.so --memfd --delay 2000000
```
`--memfd` = anonymous memfd (no disk path in maps as a normal .so file).

### `egl_install` @ `0x1d84cc`
| Hook | Mechanism | Tracked? |
|------|-----------|----------|
| `eglSwapBuffers` | `DobbyHook` only | **NO** |
| `InputConsumer::consume` | `DobbyHook` only | **NO** |
| `PC.Update` `0x8E7C40C` | Dobby + `track_hook_helper` | **YES** |
| Secondary / Tertiary / ExtraA/B | Dobby + track | YES |
| `PC.LateUpdate` `0x8E7CF50` | Dobby + track | **YES** |

### getrr / AntiCheat OnStart @ `0x8B9579C`
Install (`0x1d92b8`): `DobbyHook(OnStart, Bypass_getrr, &orig)` — **NOT tracked**.

`Bypass_getrr` @ `0x1d90b8`:
1. log `got call from getrr.`
2. iterate tracked vector → `DobbyDestroy` each
3. `blr` real OnStart via Dobby trampoline
4. reinstall each via `DobbyHook`
5. log `bypas hok result %d`

`padla` = chams AssetBundle only, **not** anti-detect.

## xxx parity

| Halalium | xxx |
|----------|-----|
| memfd inject | `xxx/inj.sh` |
| tracked Update/LateUpdate | `hhooks::install_tracked` |
| getrr destroy→call→reinstall | `hhooks::hk_getrr` (safe restore-call; no trampoline crash) |
| getrr itself untracked | `a64hook::patch_jump` only |
| egl UNTRACKED | `a64hook::install` |
| InputConsumer UNTRACKED | `a64hook::install` |
| **no GameController VMT** | TypeInfo refresh in LateUpdate + egl |
| secondary silent hooks | OFF (ESP surface) |
| gun/hit VMT | only if silent/fire |

## Inject
```sh
./AndKittyInjector --package com.axlebolt.standoff2 --libs libxxx.so --memfd --delay 2000000
```
