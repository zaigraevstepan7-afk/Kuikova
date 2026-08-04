#pragma once
// Kikaium ↔ Halalium RE bridge.
#if __has_include("sdk/generated/Offsets_generated.h")
#include "sdk/generated/Offsets_generated.h"
namespace Offsets = OffsetsGenerated;
#elif __has_include("sdk/halalium/Offsets_0.39.2.h")
#include "sdk/halalium/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#else
#error "No Halalium/Kikaium offsets header found"
#endif
