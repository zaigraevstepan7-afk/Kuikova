# Halalium (Lemming) vs xxx

**Источник:** `libhalalium.so` / Lemming (`t.me/lemminghack`, 0.39.2)  
**Не Melodium.**

## ESP Visuals (Halalium menu_body)

Enable Esp, Box, Box Type/Color, Corner Size, Health Bar, Distance/Color, Through Walls.

Melodium Name / Weapon / Ammo / Skeleton — **убраны из UI xxx**.

## Камера

`Player+0xE8 → +0x28 → +0x30 → Unity Camera → matrix@0xF0`

## Память

Halalium: in-process LDR + null.  
xxx: `hmem` LDR, без process_vm/mincore; позиции snapshot на Unity thread.

## Полоска сверху

`local | matrix | players | enemies | snap` — проверка, что чит вообще читает память.
