# Kikaium

**Kikaium** = Halalium engine (stable inject) with our branding.

## Inject this

| File | What |
|------|------|
| `kikaium/bin/libkikaium.so` | Halalium SO, rebranded — **use this** |
| `melodium/bin/libkikaium.so` | Same copy |

Rebuild from Halalium:

```bash
python3 tools/kikaium/rebrand_halalium.py \
  --src halalium/bin/libhalalium.so \
  --dst kikaium/bin/libkikaium.so
```

## What was renamed (brand only)

| Before | After |
|--------|--------|
| `Lemming` | `Kikaium` |
| `t.me/lemminghack, 0.39.2` | `t.me/kikaiumhack, 0.39.2` |
| `Halalium_Bypass` | `Kikaium_Bypass` |
| `Halalium_Hooks` | `Kikaium_Hooks` |
| `libhalalium.so` (SONAME) | `libkikaium.so` |

## What was NOT renamed (cheat functions)

Silent Aim, Anti Aim, Enable Esp, Auto Wall, Chams, Inf Ammo, Through Walls, Wallshot, …

## Our Melodium features

Halalium/Kikaium already includes the main rage/visual set. Extra Melodium toggles
(god mode, one hit kill, double tap, autostop, …) live in source:

`internal-main/internal-main/src/features/`

They are **not** binary-patched into `libkikaium.so` (closed Halalium). Port them
into a future open Kikaium build; until then inject **libkikaium.so** only — do not
stack `libmelodium.so` on top (double egl hooks / crashes).

## Download

https://github.com/zaigraevstepan7-afk/Kuikova/raw/cursor/halalium-emu-melodium-07f1/kikaium/bin/libkikaium.so
