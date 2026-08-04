#!/usr/bin/env bash
# Build Kikaium (Halalium-architecture) arm64 .so
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
  echo "======== Kikaium build #$n ========"
  cmake -S "$PROJ" -B "$PROJ/build/release-phone" \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_NDK="$NDK" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-26 \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build "$PROJ/build/release-phone" --parallel "$(nproc)"
  local so="$ROOT/bin/libkikaium.so"
  if [[ ! -f "$so" ]]; then
    # fallback copy if POST_BUILD missed
    cp -f "$PROJ/build/release-phone/libkikaium.so" "$so"
  fi
  file "$so"
  local sz
  sz=$(stat -c%s "$so")
  if [[ "$sz" -lt 500000 ]]; then
    echo "FAIL: SO too small ($sz)" >&2
    exit 2
  fi
  # NOTE: avoid `strings | grep -q` under pipefail — SIGPIPE makes the pipeline fail on match.
  local dump
  dump="$(mktemp)"
  strings -a "$so" >"$dump"
  if ! grep -qi 'kikaium' "$dump"; then
    rm -f "$dump"
    echo "FAIL: kikaium brand missing" >&2
    exit 2
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
  rm -f "$dump"
  echo "OK build #$n → $so ($sz bytes)"
}

if [[ "$VERIFY" -eq 1 ]]; then
  for i in 1 2 3; do
    # force rebuild of main objects between passes
    rm -f "$PROJ/build/release-phone/CMakeFiles/kikaium.dir/main.cpp.o" \
          "$PROJ/build/release-phone/CMakeFiles/kikaium.dir/src/features/update.cpp.o" \
          "$PROJ/build/release-phone/libkikaium.so" 2>/dev/null || true
    build_once "$i"
  done
  echo "======== ALL 3 BUILDS OK ========"
else
  build_once 1
fi
