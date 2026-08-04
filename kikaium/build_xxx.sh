#!/usr/bin/env bash
# Build xxx arm64 .so (Standoff 2 0.39.2)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJ="$ROOT/project"
NDK="${ANDROID_NDK:-/home/ubuntu/android-ndk/android-ndk-r27c}"
OUT_DIR="$(cd "$ROOT/.." && pwd)/xxx/bin"
mkdir -p "$ROOT/bin" "$OUT_DIR"

if [[ ! -d "$NDK" ]]; then
  echo "ANDROID_NDK not found: $NDK" >&2
  exit 1
fi

GEN=Unix\ Makefiles
command -v ninja >/dev/null 2>&1 && GEN=Ninja

rm -rf "$PROJ/build/release-phone"
cmake -S "$PROJ" -B "$PROJ/build/release-phone" \
  -G "$GEN" \
  -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_NDK="$NDK" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-26 \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "$PROJ/build/release-phone" --parallel "$(nproc)"

SO="$ROOT/bin/libxxx.so"
if [[ ! -f "$SO" ]]; then
  cp -f "$PROJ/build/release-phone/libxxx.so" "$SO"
fi
cp -f "$SO" "$OUT_DIR/libxxx.so"
file "$SO"
sz=$(stat -c%s "$SO")
echo "OK → $SO ($sz bytes) and $OUT_DIR/libxxx.so"

# --- verify contracts ---
dump="$(mktemp)"
strings -a "$SO" >"$dump"
grep -q 'xxx_contract:wm_click' "$dump" || { echo "FAIL: xxx contract missing"; exit 2; }
grep -q 'eglSwapBuffers' "$dump" || { echo "FAIL: egl missing"; exit 3; }
grep -q 'got call from getrr' "$dump" || { echo "FAIL: getrr bypass missing"; exit 4; }
grep -q 'DobbyHook' "$dump" || { echo "FAIL: DobbyHook strings missing"; exit 5; }
rm -f "$dump"

# Dobby must be resolved inside SO (not left UND for dynamic loader)
NDK_BIN="$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin"
if [[ -x "$NDK_BIN/llvm-readelf" ]]; then
  if "$NDK_BIN/llvm-readelf" --dyn-syms "$SO" 2>/dev/null | grep -E 'UND[[:space:]].*Dobby'; then
    echo "FAIL: unresolved Dobby* in dynsym"; exit 6
  fi
fi
# Confirm static archive was on the link line
if [[ -f "$PROJ/build/release-phone/CMakeFiles/xxx.dir/link.txt" ]]; then
  grep -q 'libdobby.a' "$PROJ/build/release-phone/CMakeFiles/xxx.dir/link.txt" \
    || { echo "FAIL: libdobby.a not linked"; exit 7; }
  grep -q 'whole-archive' "$PROJ/build/release-phone/CMakeFiles/xxx.dir/link.txt" \
    || { echo "FAIL: libdobby not whole-archive"; exit 8; }
fi

grep -q 'JNI_OnLoad' < <(readelf -Ws "$SO" 2>/dev/null) || true
echo "VERIFY OK"
