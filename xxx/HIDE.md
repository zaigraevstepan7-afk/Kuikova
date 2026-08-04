# xxx hide profile = Halalium / Lemming

Как прячется оригинальный Halalium (`libhalalium.so`, t.me/lemminghack):

## Ядро

1. **AndKitty `--memfd`** — `.so` в anonymous memfd, без пути на диске.
2. **Tracked game hooks** — Update / LateUpdate / Secondary… в реестре.
3. **getrr / AntiCheat OnStart** (`0x8B9579C`):
   - `destroy_all()` — снять tracked хуки
   - вызвать **реальный** OnStart
   - `reinstall_all()` — вернуть хуки
4. **eglSwapBuffers** и **InputConsumer::consume** — **НЕ** в tracked списке
   (иначе меню/тач умрут на каждом OnStart).

`padla` — только chams шейдеры, не антидетект.

## Что сделано в xxx

| Halalium | xxx |
|----------|-----|
| memfd inject | `xxx/inj.sh` |
| tracked Update/LateUpdate | `hook_rva_tracked` |
| getrr destroy→call→reinstall | `hhooks::hk_getrr` (safe restore-call) |
| egl UNTRACKED | `a64hook::install` |
| InputConsumer UNTRACKED | **fixed** — больше не `install_tracked` |
| getrr ON | `use_getrr_bypass=true` + `g.b_bypass` |
| secondary silent hooks | OFF (ESP build, меньше surface) |
| gun/hit VMT | только если silent/fire (ESP default: нет) |

## Инжект

```sh
./AndKittyInjector --package com.axlebolt.standoff2 --libs libxxx.so --memfd --delay 2000000
```
