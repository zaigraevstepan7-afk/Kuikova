# Halalium vs xxx — различия

## Камера / ESP matrix — СОВПАДАЕТ

```
Player + 0xE8 → +0x28 → +0x30 → Unity Camera → matrix @ 0xF0
```

Читается через `hmem::field_ptr` / `hmem::read` (in-process LDR как Halalium).

---

## Память — как читаем

| | Halalium | xxx (сейчас) |
|--|----------|--------------|
| API | in-process `LDR` + `cbz` null | `hmem::read` = memcpy после null |
| process_vm / mincore | нет | нет |
| maps | только base libunity | soft (prot/mprotect), **не гейтит** LDR |
| is_allocated | null-check | null + `>0x10000` |
| Safe types | только writes | `safe_t` / `hchain::safe_set_*` |
| Позиции ESP | Unity thread | snapshot в Update/LateUpdate |
| Матрица ESP | (FOV nest; matrix@0xF0 profile) | тот же nest + `@0xF0` на Unity thread |

---

## Совпадает

| Что | Значение |
|-----|----------|
| photon / isLocal / team / main_camera | `0x160` / `0x30` / `0x79` / `0xE8` |
| GC controls / TypeInfo | `0x2B0` / Halalium profile |
| Update / LateUpdate RVA | `0x8E7C40C` / `0x8E7CF50` |
| Tracked / untracked hooks | как Halalium |
| getrr dance | destroy → orig → reinstall |
| Through Walls | `strb player+0xD8` |

---

## Различия (остались)

| # | Halalium | xxx |
|---|----------|-----|
| 1 | egl/input = Dobby | a64 → Dobby |
| 2 | нет VMT Update/Late | VMT fallback если RVA fail |
| 3 | Visuals без Name/Weapon/Ammo | есть extras |
| 4 | полное меню | ESP + Bypass |
| 5 | watermark Lemming | **xxx** |
| 6 | Skeleton bone LDR на game tick | skeleton всё ещё может трогать bones в render (box/hp — из snapshot) |
