# nova — Standoff 2 0.39.2 internal (menu + ESP)

Greenfield build. Offsets from public AcademicDLC / hntr111 dumps:

- https://github.com/hntr111/offsets-0.39.2-64bit-standoff-2
- https://github.com/hntr111/standoff-2-offsets-x64-bit-0.39.2
- https://github.com/hntr111/standoff-2-api-offsets-0.39.2-64-bit

## Stealth inject (required)

Use AndKittyInjector hide + memfd. Without `--hide` the `.so` stays in `/proc/self/maps` and solist — easy AC signal.

```bash
./AndKittyInjector \
  --package com.axlebolt.standoff2 \
  --libs libnova.so \
  --memfd --hide \
  --delay 2000000
```

In-lib mitigations (not a ban-proof bypass):

- no logcat tags
- XOR'd sensitive strings
- prefer GOT hook over patching `libEGL` `.text`
- W^X trampolines (no RWX pages)
- no `/dev/input` scraping
- disguised worker thread name

Status strip: `nova#g` = GOT hook, `nova#i` = inline fallback.

## Build

```bash
cmake -S nova -B nova/build \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24 \
  -DANDROID_STL=c++_static
cmake --build nova/build -j
```

Output: `nova/bin/libnova.so`
