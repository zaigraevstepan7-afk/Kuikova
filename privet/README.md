# privet — Standoff 2 0.39.2 internal (from `1.zip`)

Source: user archive `1.zip` → `privet/`. ESP (box/health/distance/skeleton) + third person.

## Offset check

See [`docs/OFFSET_AUDIT.md`](docs/OFFSET_AUDIT.md). **Offset `#define` values were not changed.**

Architecture fix applied: bind TypeInfo + `il2cpp_*` API RVAs to **`libil2cpp.so`** (was incorrectly using `libunity.so`).

## Build (NDK)

```bash
export NDK=/path/to/ndk
cd privet/jni
$NDK/ndk-build -j
```

Output: `privet/bin/libpayload.so` (+ `inj`).

## Download

https://raw.githubusercontent.com/zaigraevstepan7-afk/Kuikova/cursor/privet-internal-0392-2ef8/privet/bin/libpayload.so

## Inject

```bash
./AndKittyInjector --package com.axlebolt.standoff2 --libs libpayload.so --memfd --hide --delay 2000000
```
