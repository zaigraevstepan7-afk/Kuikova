# Melodium builds (0.39.2)

| File | Notes |
|---|---|
| `libmelodium.so` | Latest arm64 build |
| `libmelodium-0.39.2-halalium-menu.so` | Same as latest — Halalium-style `eglSwapBuffers` draw + watermark open |
| `libmelodium-0.39.2-arm64.so` | Previous offsets-only build (PresentFrame path — menu did not draw) |

## Render / menu (Halalium port)

- Hook: `dlsym(libEGL.so, eglSwapBuffers)` then GOT pointer-swap (Halalium uses DobbyHook on the same symbol)
- Size: `eglQuerySurface(EGL_WIDTH/HEIGHT)` like Halalium
- Open menu: tap watermark (`##wm_click`) — brand **Lemming** / `t.me/lemminghack, 0.39.2`
- Dead path removed from init: Unity `PresentFrame` slot `0x7B5AD10`

RE notes: `halalium/extracted/MENU_WATERMARK_RE.md`
