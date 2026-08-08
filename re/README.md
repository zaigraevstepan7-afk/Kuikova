# Reverse Engineering Kit — libhalalium.so

Deep static RE of the uploaded Halalium internal (`libhalalium.so`).

## Start here

1. **[`docs/FULL_RE.md`](docs/FULL_RE.md)** — полный отчёт (что это, хуки, фичи, глобалы, лимиты)
2. **[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)** — схемы
3. **[`recovered/halalium_skeleton.cpp`](recovered/halalium_skeleton.cpp)** — восстановленный скелет потока управления
4. **[`sdk/Offsets_0.39.2.h`](sdk/Offsets_0.39.2.h)** — TypeInfo + поля под Standoff 2 **0.39.2**

## Binary

| | |
|--|--|
| Path | `bin/libhalalium.so` |
| BuildID | `12532fca99debbaa836dbbea6e5cceec95f5bbbb` |
| Version string | `t.me/lemminghack, 0.39.2` |
| Target | `com.axlebolt.standoff2` |

## Reproduce analysis

```bash
python3 re/tools/deep_re.py
python3 re/tools/fix_xrefs.py
# then the one-shot ADRP xref block in tools history, or re-run docs rebuild
```

## Verdict

Это **Dobby+ImGui internal-чит** Lemming/Halalium для SO2 0.39.2 (ESP, Silent Aim, Rage, Chams, Skin Changer, Bypass-thread, padla shaders).  
Полный исходник меню из stripped `.so` не извлечён; карта хуков/полей/глобалов — да.
