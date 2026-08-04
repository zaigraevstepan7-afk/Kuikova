# xxx hide = Halalium / Lemming (`libhalalium.so`) — Dobby parity

RE of `libhalalium.so` (t.me/lemminghack, 0.39.2). Engine: **Dobby**.

## How Halalium hides

### Inject
```sh
AndKittyInjector --package com.axlebolt.standoff2 --libs libhalalium.so --memfd --delay 2000000
```

### `egl_install`
| Hook | Mechanism | Tracked? |
|------|-----------|----------|
| `eglSwapBuffers` | `DobbyHook` only | **NO** |
| `InputConsumer::consume` | `DobbyHook` only | **NO** |
| `PC.Update` / `LateUpdate` | Dobby + track | **YES** |
| Secondary / Tertiary / ExtraA/B | Dobby + track | **YES** |

### getrr OnStart `0x8B9579C`
`DobbyHook(OnStart, Bypass_getrr, &orig)` — **NOT tracked**.

`Bypass_getrr`: destroy tracked → `blr` orig trampoline → reinstall.

## xxx = same hide for AC

| | |
|--|--|
| Engine (game/getrr) | `libdobby.a` → `DobbyHook` / `DobbyDestroy` |
| Engine (egl/input) | **a64 first** (menu-critical), Dobby fallback — still **UNTRACKED** |
| memfd | `xxx/inj.sh` |
| Update / Late / Secondary… | tracked DobbyHook |
| getrr | untracked DobbyHook + trampoline call |
| GameController VMT | none (TypeInfo) |

Watermark **xxx** top-left → tap to open menu.

```sh
./AndKittyInjector --package com.axlebolt.standoff2 --libs libxxx.so --memfd --delay 2000000
```
