#pragma once
// Standoff 2 0.39.2 (arm64) — AcademicDLC / hntr111 + Halalium/Melodium Unity RVAs

#include <cstdint>

namespace off {

// TypeInfo RVAs (relative to libil2cpp.so)
// Melodium: *(libil2cpp + TI) -> Il2CppClass*
constexpr uintptr_t kPlayerManagerTI = 180740496; // 0xAC5E190

namespace mgr {
constexpr int kStaticFields = 0x90;
constexpr int kPtr2         = 0x10;
constexpr int kPtr3         = 0x0;
constexpr int kLocal        = 0x70;
constexpr int kList         = 0x28;
constexpr int kListSize     = 0x20;
}

namespace player {
constexpr int kTeam             = 0x79;
constexpr int kMainCameraHolder = 0x28;
constexpr int kCharacterView    = 0x48;
constexpr int kWeaponry         = 0x88;
constexpr int kMovement         = 0x98;
constexpr int kArmsLod          = 0x120; // SkinnedMeshLodGroup / gloves
constexpr int kCharacterLod     = 0x128; // CharacterLOD → skinned mesh
constexpr int kPhoton           = 0x160;
constexpr int kVisible          = 0xD8;
}

namespace lod {
constexpr int kSkinnedMesh = 0x30; // _meshRenderer / _glovesMeshRenderer
}

namespace photon {
constexpr int kName    = 0x20;
constexpr int kIsLocal = 0x30;
}

// libunity method RVAs (Halalium / Melodium 0.39.2)
namespace unity {
constexpr uintptr_t kPcUpdate     = 0x8E7C40C;
constexpr uintptr_t kPcLateUpdate = 0x8E7CF50;
constexpr uintptr_t kShaderFind   = 0x6A95144;
constexpr uintptr_t kMatCtor      = 0x6A98518;
constexpr uintptr_t kMatSetColor  = 0x6A96904;
constexpr uintptr_t kMatSetFloat  = 0x6A8BF28;
constexpr uintptr_t kMatSetInt    = 0x6A84BE4;
constexpr uintptr_t kRendererSet  = 0x6A91498;
}

// libil2cpp API RVAs (api_0.39.2.txt)
namespace api {
constexpr uintptr_t kDomainGet           = 0x5C86D54;
constexpr uintptr_t kDomainAssemblyOpen  = 0x5C86D58;
constexpr uintptr_t kAssemblyGetImage    = 0x5C86BFC;
constexpr uintptr_t kClassFromName       = 0x9D5B510;
constexpr uintptr_t kObjectNew           = 0x5C86EF8;
constexpr uintptr_t kStringNew           = 0x5C86F3C;
constexpr uintptr_t kThreadAttach        = 0x5C86F50;
}

} // namespace off
