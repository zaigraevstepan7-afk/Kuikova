# Halalium emulator — Melodium update toolkit
#
# One-command update when a new Halalium drops:
#
#   python3 tools/halalium_emu/halalium_emu.py profile \
#     --so /path/to/libhalalium.so \
#     --script /path/to/script.json \
#     --dump /path/to/dump.cs \
#     --out tools/halalium_emu/out \
#     --apply
#
#   # then rebuild Melodium (release-phone) and ship melodium/bin/libmelodium.so
#
# Commands:
#   profile   — RE Halalium SO → profile.json + Offsets_generated.h
#   apply     — write generated offsets into Melodium/halalium sdk trees
#   diff      — compare two Halalium SOs (features/strings/hooks)
#   emu-check — assert Melodium still follows Halalium render/menu contract
#
# Melodium architecture contract (emulated Halalium):
#   1. Render via eglSwapBuffers (NOT Unity PresentFrame)
#   2. Menu open via ##watermark / ##wm_click
#   3. Offsets from Offsets_generated.h / OffsetsBridge.h
#   4. Keep Melodium feature code (ESP/aim/chams/…) — Halalium is the map, not the product
