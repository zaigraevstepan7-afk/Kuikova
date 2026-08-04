# Kikaium = Halalium RE + our Melodium-style source

Primary product: **Kikaium** built from source (`kikaium/project`), mapped from Halalium reverse.

```bash
# Full RE + offsets refresh
bash tools/halalium_emu/update.sh \
  halalium/bin/libhalalium.so \
  okak/okaakka/script.json \
  "okak/okaakka/dump (1).cs"

# Build (verify 3x)
bash kikaium/build.sh --verify
# → kikaium/bin/libkikaium.so
```

Docs: `kikaium/README.md`, `kikaium/docs/HALALIUM_FULL_RE.md`.
