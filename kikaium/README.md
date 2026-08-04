# Kikaium

**Kikaium** = Halalium engine with our brand + renamed menu chrome.

## Inject

https://github.com/zaigraevstepan7-afk/Kuikova/raw/cursor/halalium-emu-melodium-07f1/kikaium/bin/libkikaium.so

Watermark must show **`Kikaium`** and **`Kikaium | private | 0392`** — if you still see Lemming, you have an old SO cached; re-download and force-stop the game.

## Brand

| Before | After |
|--------|--------|
| Lemming | Kikaium |
| t.me/lemminghack, 0.39.2 | Kikaium \| private \| 0392 |
| Halalium_Bypass / Hooks | Kikaium_Bypass / Hooks |
| libhalalium.so | libkikaium.so |

## Menu tabs (so it is not a Halalium clone)

| Before | After |
|--------|--------|
| Rage | Nova |
| Visuals | Drawing |
| Misc | More |
| Settings | Controls |
| Skins | Items |
| Weapons | Loadout |
| World | Scene |
| Watermark | Hud Label |

Cheat labels kept: Silent Aim, Anti Aim, Enable Esp, …

```bash
python3 tools/kikaium/rebrand_halalium.py
```
