# Halalium (Lemming) vs xxx

**Источник:** `libhalalium.so` / Lemming (`t.me/lemminghack`, 0.39.2)  
**Не Melodium.**

## ESP Visuals (Halalium menu_body)

Enable Esp, Box, Box Type/Color, Corner Size, Health Bar, Distance/Color, Through Walls.

Melodium Name / Weapon / Ammo / Skeleton — **убраны из UI xxx**.

## Камера / W2S

FOV nest (Halalium Update): `Player+0xE8 → +0x28 → +0x30 → Unity Camera*`  
ESP matrix (как Melodium, для `world2screen`): `PlayerMainCamera+0x20 → +0x10 → @0xF0`  
Fallback: `projection * worldToCamera` через Injected.

## Память

Halalium: in-process LDR + null.  
xxx: `hmem` LDR, без process_vm/mincore; позиции snapshot на Unity thread.

## Полоска сверху+снизу

`sdk | local | matrix(stage) | players | enemies | snap | f` — проверка, что чит читает память и EGL жив.
