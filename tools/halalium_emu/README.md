# Halalium emulator — Kikaium update toolkit

One-command update when a new Halalium drops:

```bash
bash tools/halalium_emu/update.sh \
  /path/to/libhalalium.so \
  /path/to/script.json \
  /path/to/dump.cs

bash kikaium/build.sh --verify
```

## Commands

| Cmd | Role |
|-----|------|
| `profile` | RE Halalium SO → profile.json + Offsets_generated.h |
| `apply` | write generated offsets into **Kikaium** (+ legacy Melodium) trees |
| `diff` | compare two Halalium SOs |
| `emu-check` | assert Kikaium still follows Halalium contract |

## Improved RE (v2)

- Brace-balanced `dump.cs` class scrape (nested types)
- ScriptMethod method RVAs (`getrr` OnStart, …)
- Extra TypeInfos: GunController, PlayerMainCamera
- Expanded field map (aim/hit/movement/PlayerManager slots)
- 23 Halalium feature UI labels

## Kikaium architecture contract

1. Render via `eglSwapBuffers` (NOT Unity PresentFrame)
2. Menu open via `##wm_click`
3. Offsets from `Offsets_generated.h` / `OffsetsBridge.h`
4. VMT Update hooks + ESP/visual overlays when `g_sdk_ready`
5. Melodium-style feature code — Halalium is the map, Kikaium is the product
