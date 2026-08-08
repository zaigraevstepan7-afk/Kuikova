# Halalium reverse / offset restore

Working Standoff 2 **0.39.2** internal kit analysis + restored offset header.

- **Docs:** [docs/HALALIUM_RE.md](docs/HALALIUM_RE.md) — pipeline, RE findings, Melodium dead-features checklist  
- **Offsets:** [sdk/Offsets_0.39.2.h](sdk/Offsets_0.39.2.h)  
- **Remap tool:** [tools/map_offsets.py](tools/map_offsets.py)  
- **Binaries (renamed):** [bin/](bin/) — `inj.sh`, `AndKittyInjector`, `libhalalium.so`, `padla`  
- **Extracted:** [extracted/](extracted/) — padla shaders, disasm snippets, community offset copies  

## Deploy (device)

```text
padla                         -> /sdcard/Android/data/com.axlebolt.standoff2/files/padla
AndKittyInjector libhalalium.so inj.sh -> /data/local/tmp/
sh /data/local/tmp/inj.sh
```
