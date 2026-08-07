# nova — Standoff 2 0.39.2 internal (menu + ESP)

Greenfield build. Offsets from public AcademicDLC / hntr111 dumps (internet):

- https://github.com/hntr111/offsets-0.39.2-64bit-standoff-2
- https://github.com/hntr111/standoff-2-offsets-x64-bit-0.39.2
- https://github.com/hntr111/standoff-2-api-offsets-0.39.2-64-bit

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

## Inject (root)

```bash
./AndKittyInjector --package com.axlebolt.standoff2 --libs libnova.so --memfd --delay 2000000
```
