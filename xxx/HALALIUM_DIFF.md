# Halalium vs xxx — различия

## Камера / ESP matrix — СОВПАДАЕТ

```
Player + 0xE8 → +0x28 → +0x30 → Unity Camera → matrix @ 0xF0
```

В `esp.cpp::cache_matrix()` — единственный путь (без Melodium).

---

## Совпадает

| Что | Значение |
|-----|----------|
| photon / isLocal / team / main_camera | `0x160` / `0x30` / `0x79` / `0xE8` |
| GC controls / TypeInfo | `0x2B0` / Halalium profile |
| Update / LateUpdate RVA | `0x8E7C40C` / `0x8E7CF50` |
| Tracked: Update, Late, Secondary, Tertiary, Extra | Dobby + hhooks |
| Untracked: egl, InputConsumer, getrr | да |
| getrr dance | destroy → orig → reinstall |
| ESP Box / Corner / HP / Distance | да |
| Inject | `--memfd --delay 2000000` |

---

## Различия

| # | Halalium | xxx | Важно |
|---|----------|-----|--------|
| 1 | Through Walls → `player+0xD8` | **добавлено** в меню, default ON + при ESP | закрыто |
| 2 | egl/input = Dobby | a64 → Dobby → GOT | hide fingerprint |
| 3 | нет VMT на Update/Late | VMT fallback если RVA fail | hide |
| 4 | Visuals: Box/HP/Distance | + Name / Weapon / Ammo / Skeleton | Melodium extras |
| 5 | Rage/Visuals/Misc/Skins | только ESP + Bypass | продукт |
| 6 | watermark Lemming | watermark **xxx** | бренд |
| 7 | Secondary/Extra полные | часть stub | aim later |
| 8 | Distance от PMC nest | Distance от `MainCameraHolder` | мелочь |

---

## Melodium leftover в xxx (Halalium нет)

- Name / Weapon / Ammo ESP (в UI)
- Update Matrix checkbox (мёртвый)
- Skeleton отдельным тогглом
- fog/duck/DT/hitmarkers в коде (выкл)
- a64 hook engine для egl/input
- copper Melodium-style тема
