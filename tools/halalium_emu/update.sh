#!/usr/bin/env bash
# Drop-in update helper: new Halalium SO + dumps → Kikaium offsets.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SO="${1:-$ROOT/halalium/bin/libhalalium.so}"
SCRIPT="${2:-$ROOT/okak/okaakka/script.json}"
DUMP="${3:-}"
if [[ -z "$DUMP" ]]; then
  if [[ -f "$ROOT/okak/okaakka/dump (1).cs" ]]; then
    DUMP="$ROOT/okak/okaakka/dump (1).cs"
  elif [[ -f "$ROOT/okak/okaakka/dump.cs" ]]; then
    DUMP="$ROOT/okak/okaakka/dump.cs"
  fi
fi

ARGS=(profile --so "$SO" --out "$ROOT/tools/halalium_emu/out" --apply)
[[ -f "$SCRIPT" ]] && ARGS+=(--script "$SCRIPT")
[[ -n "${DUMP:-}" && -f "$DUMP" ]] && ARGS+=(--dump "$DUMP")

python3 "$ROOT/tools/halalium_emu/halalium_emu.py" "${ARGS[@]}"
python3 "$ROOT/tools/halalium_emu/halalium_emu.py" emu-check
echo "[ok] offsets applied to Kikaium. Next: bash kikaium/build.sh --verify"
