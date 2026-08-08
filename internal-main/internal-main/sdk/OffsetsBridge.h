#pragma once
// Melodium ↔ Halalium emulator bridge.
// Prefer generated header when present; fall back to hand-curated 0.39.2 table.
#if __has_include("sdk/generated/Offsets_generated.h")
#include "sdk/generated/Offsets_generated.h"
namespace Offsets = OffsetsGenerated;
#elif __has_include("../../../melodium/sdk/Offsets_0.39.2.h")
#include "../../../melodium/sdk/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#else
#include "../../../halalium/sdk/Offsets_0.39.2.h"
namespace Offsets = Offsets0392;
#endif
