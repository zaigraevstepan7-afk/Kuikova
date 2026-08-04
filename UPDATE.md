# UPDATE — Kikaium (Halalium engine)

Primary product is now **Kikaium** = Halalium `.so` with brand strings renamed.

```bash
# Drop new Halalium SO, then:
cp /path/to/libhalalium.so halalium/bin/libhalalium.so
python3 tools/kikaium/rebrand_halalium.py
# → kikaium/bin/libkikaium.so
```

Cheat feature names are left untouched. Melodium source under `internal-main/`
remains for porting extras (god mode / OHK / DT / …) later — do not inject
`libmelodium.so` together with `libkikaium.so`.

Offset emulator (optional, for Melodium source):

```bash
bash tools/halalium_emu/update.sh \
  halalium/bin/libhalalium.so \
  okak/okaakka/script.json \
  "okak/okaakka/dump (1).cs"
```
