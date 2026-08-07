# nova — Standoff 2 0.39.2 internal (menu + Halalium/Lemming chams)

Greenfield build. ESP removed — chams via Unity Material on CharacterLOD
(same path as Halalium / Lemming / Melodium):

- `Shader.Find` → `Material` ctor → color / `_ZTest=8` / `_ZWrite=0`
- `Renderer.set_material` on `player+0x128` → `+0x30` skinned mesh
- Applied on Unity thread via `PlayerController.Update` (`libunity+0x8E7C40C`)

Offsets from public AcademicDLC / hntr111 + Melodium Unity RVAs.

## Inject

```bash
./AndKittyInjector \
  --package com.axlebolt.standoff2 \
  --libs libnova.so \
  --memfd --hide \
  --delay 2000000
```

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

Status: `nova#i | ok | ch:a64/ok | a:N` — chams hook live, materials applied.
