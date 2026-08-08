# Kikaium — Halalium reconstruction (Melodium-quality UI)

Internal cheat rebuilt from **Halalium binary RE**, not a string-rename of `libhalalium.so`.

| | Halalium binary | Kikaium product |
|--|--|--|
| Size | ~2.58 MB | ~1.1 MB (our sources) |
| Hook engine | Dobby | a64 tracked tramp (Dobby-equiv) |
| Update | RVA `0x8E7C40C` | same RVA + VMT fallback |
| Menu | WildRage tabs + ##wm_click | same IA / Halalium feature labels |
| Brand | Lemming | Kikaium |

## Build

```bash
bash tools/halalium_emu/update.sh
bash kikaium/build.sh --verify
# → kikaium/bin/libkikaium.so
```

## Architecture (from Halalium disasm)

```text
inject → ctor/JNI_OnLoad → process-once
  → eglSwapBuffers hook (ImGui + overlays)
  → resolve libunity base (Halalium string)
  → Dobby-style RVA: Update 0x8E7C40C + LateUpdate 0x8E7CF50
  → VMT fallback GameController/Hit/Gun
  → menu: Rage / Visuals / Misc / Settings / Skins
```

Docs: `kikaium/docs/HALALIUM_FULL_RE.md`, `halalium/extracted/`.
