#pragma once
// Standoff 2 0.39.2 (arm64) — public AcademicDLC / hntr111 offsets
// Sources:
//   https://github.com/hntr111/offsets-0.39.2-64bit-standoff-2
//   https://github.com/hntr111/standoff-2-offsets-x64-bit-0.39.2
//   https://github.com/hntr111/standoff-2-api-offsets-0.39.2-64-bit

#include <cstdint>

namespace off {

// TypeInfo RVAs (relative to libil2cpp.so)
// Melodium: *(libil2cpp + TI) -> Il2CppClass*
constexpr uintptr_t kPlayerManagerTI = 180740496; // 0xAC5E190

namespace mgr {
constexpr int kStaticFields = 0x90; // Il2CppClass.static_fields
constexpr int kPtr2         = 0x10;
constexpr int kPtr3         = 0x0;
constexpr int kLocal        = 0x70;
constexpr int kList         = 0x28;
constexpr int kListSize     = 0x20;
}

namespace list {
constexpr int kBuffer = 0x18;
constexpr int kEntry  = 0x30;
constexpr int kStride = 0x18;
}

namespace player {
constexpr int kTeam              = 0x79;
constexpr int kCharacterView     = 0x48;
constexpr int kWeaponry          = 0x88;
constexpr int kMovement          = 0x98;
constexpr int kMainCamera        = 0xE8;
constexpr int kMainCameraHolder  = 0x28;
constexpr int kPhoton            = 0x160;
}

namespace cam {
constexpr int kTransform = 0x20;
constexpr int kPtr       = 0x10;
constexpr int kMatrix    = 0xF0;
}

namespace xform {
constexpr int kData     = 0xB0;
constexpr int kPosition = 0x44;
}

namespace biped {
constexpr int kBipedMap        = 0x48; // on CharacterView
constexpr int kHead            = 0x20;
constexpr int kHip             = 0x88;
constexpr int kTransformObject = 0x10;
}

namespace photon {
constexpr int kName = 0x20;
}

// Il2CppClass / API layout from api dump
namespace il2cpp {
constexpr int kStaticFields = 0x90;
constexpr uintptr_t kDomainGet      = 0x5C86D54;
constexpr uintptr_t kThreadAttach   = 0x5C86F50;
constexpr uintptr_t kClassFromName  = 0x9D5B510;
constexpr uintptr_t kRuntimeInvoke  = 0x5C86F28;
}

} // namespace off
