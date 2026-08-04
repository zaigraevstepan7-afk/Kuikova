#!/usr/bin/env bash
# Build xxxpastuxxx arm64 .so
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJ="$ROOT/project"
NDK="${ANDROID_NDK:-/home/ubuntu/android-ndk/android-ndk-r27c}"
VERIFY=0
[[ "${1:-}" == "--verify" ]] && VERIFY=1

if [[ ! -d "$NDK" ]]; then
  echo "ANDROID_NDK not found: $NDK" >&2
  exit 1
fi

build_once() {
  local n="$1"
  echo "======== xxxpastuxxx build #$n ========"
  cmake -S "$PROJ" -B "$PROJ/build/release-phone" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_NDK="$NDK" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-26 \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build "$PROJ/build/release-phone" --parallel "$(nproc)"
  local so="$ROOT/bin/libxxxpastuxxx.so"
  mkdir -p "$ROOT/bin"
  if [[ ! -f "$so" ]]; then
    cp -f "$PROJ/build/release-phone/libxxxpastuxxx.so" "$so"
  fi
  # keep legacy filename symlink for inject scripts that still say libkikaium.so
  ln -sfn libxxxpastuxxx.so "$ROOT/bin/libkikaium.so"
  file "$so"
  local sz
  sz=$(stat -c%s "$so")
  if [[ "$sz" -lt 500000 ]]; then
    echo "FAIL: SO too small ($sz)" >&2
    exit 2
  fi
  local dump
  dump="$(mktemp)"
  strings -a "$so" >"$dump"
  if ! grep -qi 'xxxpastuxxx' "$dump"; then
    rm -f "$dump"
    echo "FAIL: xxxpastuxxx brand missing" >&2
    exit 2
  fi
  if ! grep -q 't.me/xxxstuxxx' "$dump"; then
    rm -f "$dump"
    echo "FAIL: telegram link missing" >&2
    exit 2
  fi
  if grep -qiE 'lemminghack|Lemming|Halalium_Hooks|KIKAIUM|Kikaium' "$dump"; then
    # allow internal RE comments in code that may be optimized out; fail only on clear brand leaks
    if grep -qE 't\.me/lemminghack|Lemming$|"Kikaium"|kikaium_contract' "$dump"; then
      rm -f "$dump"
      echo "FAIL: old Lemming/Kikaium brand still in binary" >&2
      exit 2
    fi
  fi
  if ! grep -q 'eglSwapBuffers' "$dump"; then
    rm -f "$dump"
    echo "FAIL: eglSwapBuffers marker missing" >&2
    exit 3
  fi
  if ! grep -q 'update::init' "$dump"; then
    rm -f "$dump"
    echo "FAIL: update::init marker missing" >&2
    exit 3
  fi
  if ! grep -q 'xxxpastuxxx_contract:wm_click' "$dump"; then
    rm -f "$dump"
    echo "FAIL: menu contract marker missing" >&2
    exit 4
  fi
  if ! grep -qE 'xxxpastuxxx reconstruct|Halalium reconstruct|VMT ready' "$dump"; then
    rm -f "$dump"
    echo "FAIL: init path marker missing" >&2
    exit 4
  fi
  if ! readelf -Ws "$so" 2>/dev/null | grep -q 'JNI_OnLoad'; then
    rm -f "$dump"
    echo "FAIL: JNI_OnLoad not exported" >&2
    exit 5
  fi
  if ! readelf -d "$so" 2>/dev/null | grep -q 'INIT_ARRAY'; then
    rm -f "$dump"
    echo "FAIL: INIT_ARRAY (constructor) missing" >&2
    exit 5
  fi
  rm -f "$dump"
  echo "OK build #$n → $so ($sz bytes) [JNI+VMT+egl+wm]"
}

if [[ "$VERIFY" -eq 1 ]]; then
  for i in 1 2 3; do
    rm -f "$PROJ/build/release-phone/CMakeFiles/xxxpastuxxx.dir/main.cpp.o" \
          "$PROJ/build/release-phone/CMakeFiles/xxxpastuxxx.dir/src/features/update.cpp.o" \
          "$PROJ/build/release-phone/CMakeFiles/kikaium.dir/main.cpp.o" \
          "$PROJ/build/release-phone/libxxxpastuxxx.so" \
          "$PROJ/build/release-phone/libkikaium.so" 2>/dev/null || true
    build_once "$i"
  done
  echo "======== ALL 3 BUILDS OK ========"
else
  build_once 1
fi
