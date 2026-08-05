# xxx — Standoff 2 internal (0.39.2)

База: **Halalium / Lemming** (`t.me/lemminghack`, 0.39.2) — **не Melodium**.

## Сейчас (только ESP + bypass)

| Фича | Как в Halalium |
|------|----------------|
| Enable Esp / Box / Box Type / Health / Distance / Through Walls | да |
| Name / Weapon / Ammo / Skeleton UI | **нет** (это Melodium) |
| Bypass getrr | да |
| Aim / chams / skins / rage | нет (позже) |
| Полоска сверху+снизу (players/enemies/matrix) | да — проверка чтения памяти |

## Инжект

```sh
./AndKittyInjector --package com.axlebolt.standoff2 --libs libxxx.so --memfd --delay 2000000
```

Либа: `xxx/bin/libxxx.so`

## Сборка

```bash
export ANDROID_NDK=/home/ubuntu/android-ndk/android-ndk-r27c
bash kikaium/build_xxx.sh
```

## Диагностика

Полоска **сверху и снизу** (каждый кадр):
`xxx | sdk:OK/NO | local:OK/NO | matrix:OK/NO(stage) | players:N | enemies:M | snap:K | f:F`

- `f` растёт → EGL/ImGui жив
- `sdk:OK` + `local:OK` + `players>0` → хуки Update читают игроков
- `matrix:OK` → W2S матрица (`pmc+0x20→+0x10→@0xF0` или P×V)
- `enemies/snap>0` + ESP on → боксы должны рисоваться

См. также `xxx/HALALIUM_DIFF.md`, `xxx/HIDE.md`.
