# privet — Standoff 2 0.39.2 internal (from `1.zip`)

Source restored as in original archive. ESP (box/health/distance/skeleton) + third person.

Offset check (values unchanged): [`docs/OFFSET_AUDIT.md`](docs/OFFSET_AUDIT.md).

## Build

```bash
export NDK=/path/to/ndk
cd privet/jni
$NDK/ndk-build -j
```

## Download

https://raw.githubusercontent.com/zaigraevstepan7-afk/Kuikova/cursor/privet-internal-0392-2ef8/privet/bin/libpayload.so

## Inject

```bash
./AndKittyInjector --package com.axlebolt.standoff2 --libs libpayload.so --memfd --hide --delay 2000000
```
