# Melodium = Halalium architecture + our features

Melodium is **not** a full decompile of Halalium. It is a Halalium-compatible shell:

| Halalium (binary) | Melodium (source) |
|---|---|
| `dlsym` + `DobbyHook(eglSwapBuffers)` | `dlsym` + GOT pointer-swap of `eglSwapBuffers` |
| Menu open via `##wm_click` on watermark | same |
| Watermark `Lemming` / `t.me/lemminghack, 0.39.2` | same branding |
| Offsets buried in LDR / runtime | `Offsets_generated.h` from emulator |
| Closed features | Melodium ESP / silent / AA / chams / world / misc |
| In-process `LDR`/`STR` (+ maps null checks) | `includes/halalium_mem.h` + `egl/memory.cpp` (no `process_vm_*`) |

## Memory (Halalium-style)

Halalium does **not** use `process_vm_readv` / `process_vm_writev`. After inject it reads/writes the game address space directly (`ldr [base, TypeInfo]`, `ldr [player, #0x160]`, `strb …`) with null / maps checks.

Melodium mirrors that via `hmem::read` / `hmem::write` / `hmem::typeinfo` / `hmem::field`. The old `memory::` API is a thin wrapper over the same path.

## Update (no more hand-pain)

When a new Halalium drops + you have a fresh dump:

```bash
# 1) put new SO
cp /path/to/libhalalium.so halalium/bin/libhalalium.so

# 2) put dumps (script.json + dump.cs) under okak/okaakka/ or pass paths
bash tools/halalium_emu/update.sh \
  halalium/bin/libhalalium.so \
  okak/okaakka/script.json \
  "okak/okaakka/dump (1).cs"

# 3) rebuild Melodium release-phone → melodium/bin/libmelodium.so
```

What the emulator does:

1. Profiles Halalium SO (strings, egl install xrefs, features, watermark)
2. Pulls TypeInfo from `script.json` (exact FQ names, not substring traps)
3. Merges Halalium-confirmed fields + dump.cs scrapes
4. Writes `Offsets_generated.h` into Melodium / melodium/sdk / halalium/sdk
5. `emu-check` asserts Melodium still follows the Halalium render/menu contract

## Diff two Halalium builds

```bash
python3 tools/halalium_emu/halalium_emu.py diff \
  --old old/libhalalium.so --new new/libhalalium.so
```

## Why this instead of rewriting from Halalium RE

Halalium is stripped. Full source recovery is a dead end. This path keeps **our** feature code and treats Halalium as the **map** that regenerates every patch.
