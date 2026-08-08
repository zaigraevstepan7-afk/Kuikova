# Halalium — menu draw / open / watermark (from `libhalalium.so`)

Extracted from uploaded binary (BuildID `12532fca…`, version string `t.me/lemminghack, 0.39.2`).

## Render path (NOT PresentFrame)

Halalium does **not** swap Unity `GfxDeviceGLES::PresentFrame`.

Install @ `0x1d84cc` (`eglSwapBuffers.asm`):

```text
x0 = "libEGL.so"          @ 0x36373
x1 = "eglSwapBuffers"     @ 0x3575b
bl  dlsym-wrapper         @ 0x26c550  (GOT -> dlsym)
cbz x0, skip
x1 = hook_callback        @ 0x1d76f0
x2 = &orig_eglSwapBuffers @ 0x279578
bl  DobbyHook-wrapper     @ 0x26c560  (GOT -> DobbyHook)
```

Also hooks `android::InputConsumer::consume` (`libinput.so`) @ callback `0x1d760c` for input.

## Hook callback `0x1d76f0`

Per-frame:

1. One-time ImGui + `imgui_impl_opengl3` init
2. `eglQuerySurface` width/height (`0x3057` / `0x3056`)
3. ImGui NewFrame / touch / features / menu
4. `ImGui_ImplOpenGL3_RenderDrawData`
5. tail-call original `eglSwapBuffers`

## Menu open

Primary toggle is **click on watermark**, not Insert/RightAlt.

- Invisible hit target string: `##wm_click` @ `0x2f52d`
- Xref @ `0x1db464` inside watermark draw
- On click: `byte[0x279064] ^= 1` (menu open flag)
- Menu body @ `0x1db864` gated: `if (!flag) skip` (`cbz` → `0x1dcd24`)

`Insert` / `RightAlt` strings exist only as ImGui key-name table entries (no code xrefs) — not the menu hotkey path.

## Watermark

| Item | VA / note |
|---|---|
| Window id | `##watermark` @ `0x34fc6` |
| Begin flags | `0x30011AF` → NoTitleBar\|NoResize\|NoMove\|NoScrollbar\|NoCollapse\|NoBackground\|NoSavedSettings\|NoFocusOnAppearing + high `0x300` |
| Brand | `Lemming` @ `0x2e015` |
| Version line | `t.me/lemminghack, 0.39.2` @ `0x2d17f` |
| Extra credit | `from hehket: real thank u` @ `0x36646` |
| Settings panel | `##settings_watermark` @ `0x33443`, label `Watermark` |
| Click id | `##wm_click` toggles open flag |

Watermark is always drawn; full menu chrome only when open flag is set.

## Melodium mapping

| Halalium | Melodium |
|---|---|
| `dlsym` + `DobbyHook(eglSwapBuffers)` | `dlsym` + GOT/`JUMP_SLOT` swap (same call site as Unity) or inline interceptor |
| watermark click → open | `##wm_click` → `open = !open` |
| `Lemming` / `t.me/lemminghack, 0.39.2` | same branding on overlay watermark |
| dead `PresentFrame` slot | **do not use** `0x7B5AD10` |
