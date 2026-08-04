# Melodium feature extras (to port onto Kikaium)

`libkikaium.so` already ships Halalium’s built-in cheats (Silent Aim, Anti Aim,
ESP, Chams, Auto Wall, Inf Ammo, …).

These Melodium UI toggles are **ours** and still only in source
(`internal-main/internal-main/src/features/` + menu):

- god mode
- one hit kill
- double tap
- endless ammo / fire rate
- autostop / auto strafer / auto fire
- fast walk / duck / jitter / predict
- third person / remove scope / aspect ratio
- set health
- Melodium ESP details (box/name/health/ammo/skeleton/weapon)
- world / fog / sky / solid world
- tracer / marker / damage marker
- weapon chams variants

Do not stack `libmelodium.so` on `libkikaium.so` (double hooks). Port these
into an open Kikaium build when ready.
