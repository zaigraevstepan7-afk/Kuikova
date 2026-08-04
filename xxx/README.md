# xxx — Standoff 2 internal (0.39.2)

Новый чит **xxx** под SO2 **0.39.2** (arm64).

## Что внутри (сейчас)

| Фича | Статус |
|------|--------|
| Меню (watermark `xxx`, клик открывает) | да |
| ESP (box / name / hp / skeleton / weapon / ammo / distance) | да, по умолчанию ON |
| Bypass (getrr OnStart safe restore-call) | да, по умолчанию ON |
| Aim / chams / skins / rage | нет (добавим позже) |

## Инжект (root + AndKitty) — OnePlus Ace 6

На телефон в `/data/local/tmp/`:

```text
AndKittyInjector
libxxx.so
inj.sh
```

```sh
su
cd /data/local/tmp
chmod 755 AndKittyInjector inj.sh
sh inj.sh
```

Или руками:

```sh
./AndKittyInjector --package com.axlebolt.standoff2 --libs libxxx.so --memfd --delay 2000000
```

Запуск: зайди в лобби/матч → тап по watermark **xxx** слева сверху → меню.

## Сборка

```bash
export ANDROID_NDK=/home/ubuntu/android-ndk/android-ndk-r27c
bash kikaium/build_xxx.sh
# → xxx/bin/libxxx.so
```

## Дампы

`dumpmmm.7z` на GitHub проверен: TypeInfo совпадают с `Offsets_0.39.2.h` (см. `xxx/TYPEINFO_0.39.2.txt`).

## Исходники

Логика чита: `kikaium/project/` (egl hook, ESP, getrr bypass, меню xxx).
