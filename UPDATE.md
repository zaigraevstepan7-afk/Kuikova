# Kikaium = Halalium RE + Melodium-style source (real cheat SO)

Primary product: **Kikaium** (`kikaium/project` → `kikaium/bin/libkikaium.so`).

Unlike the soft menu-only Melodium snapshot, Kikaium builds with:
- VMT Update/LateUpdate + hit/gun hooks
- ESP / visual overlays on `eglSwapBuffers` after `g_sdk_ready`
- Halalium offsets from improved `tools/halalium_emu` (brace-aware dump.cs + ScriptMethod)

```bash
bash tools/halalium_emu/update.sh
bash kikaium/build.sh --verify   # 3× build + JNI/VMT/egl/wm checks
```

Docs: `kikaium/README.md`, `kikaium/docs/HALALIUM_FULL_RE.md`.
