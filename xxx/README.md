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

Сверху полоска:
`xxx | Halalium/Lemming | local:OK/NO | matrix:OK/NO | players:N | enemies:M | snap:K`

- `players>0` + `local:OK` → память/хуки Update читают игроков
- `matrix:OK` → камера `+0xE8→+0x28→+0x30→@0xF0` ок
- `enemies/snap>0` → ESP должен рисовать боксы

См. также `xxx/HALALIUM_DIFF.md`, `xxx/HIDE.md`.
