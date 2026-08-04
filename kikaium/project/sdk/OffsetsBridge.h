#pragma once
// Kikaium ↔ Halalium RE bridge.
// Prefer emu-generated offsets; fall back to curated Halalium 0.39.2 table.
#if __has_include("sdk/generated/Offsets_generated.h")
#include "sdk/generated/Offsets_generated.h"
namespace Offsets = OffsetsGenerated;
#elif __has_include("sdk/halalium/Offsets_0.39.2.h")
#include "sdk/halalium/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#elif __has_include("../sdk/Offsets_0.39.2.h")
#include "../sdk/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#else
#error "No Halalium/Kikaium offsets header found"
#endif
