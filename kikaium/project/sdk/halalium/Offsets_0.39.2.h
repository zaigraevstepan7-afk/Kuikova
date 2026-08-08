// Halalium / Standoff 2 — restored offsets for game version 0.39.2 (arm64)
// Sources:
//   - libhalalium.so reverse (field LDR usage, hook chains)
//   - okaakka dump: dump.cs + script.json TypeInfo
//   - community offsets-0.39.2-64bit.h (hntr111) cross-check
//   - api_0.39.2.txt Il2CppClass layout (static_fields @ 0x90)
//
// TypeInfo addresses are RVAs: *(libil2cpp_base + TypeInfo) -> Il2CppClass*
// Resolve singleton:
//   klass = *(base + TypeInfo)
//   static_fields = *(klass + Il2Cpp.static_fields)   // 0x90 on 0.39.2
//   instance = *(static_fields + static_field_offset) // often 0x0 or 0x10

#pragma once
#include <cstdint>

namespace Offsets0392 {

// ---------------------------------------------------------------------------
// Il2CppClass / MethodInfo layout (from api_0.39.2.txt)
// ---------------------------------------------------------------------------
namespace Il2Cpp {
    constexpr uintptr_t klass_parent        = 0x40;
    constexpr uintptr_t klass_namespace     = 0xB0;
    constexpr uintptr_t klass_name          = 0xF8;
    constexpr uintptr_t klass_static_fields = 0x90; // NOT 0x60 (older layouts)
    constexpr uintptr_t klass_vtable        = 0x1B8;

    constexpr uintptr_t method_methodPointer        = 0x8;
    constexpr uintptr_t method_invokerMethod        = 0x20;
    constexpr uintptr_t method_virtualMethodPointer = 0x30;
    constexpr uintptr_t method_name                 = 0x40;
}

// ---------------------------------------------------------------------------
// TypeInfo RVAs (script.json / okaakka) — add to libil2cpp base
// ---------------------------------------------------------------------------
namespace TypeInfo {
    constexpr uintptr_t PlayerManager     = 0xAC5E190; // 180740496
    constexpr uintptr_t GameController    = 0xAC58BB0; // 180718512
    constexpr uintptr_t PhotonNetwork     = 0xAC5DE18; // 180739608
    constexpr uintptr_t BombManager       = 0xAC4FAC0; // 180681408
    constexpr uintptr_t InventoryManager  = 0xAC5C018; // 180731928
    constexpr uintptr_t PlayerControls    = 0xAC5E0E0; // 180740320
    constexpr uintptr_t PlayerController  = 0xAC5E0D8; // 180740312
    constexpr uintptr_t WeaponController  = 0xAC61A18; // 180754968
    constexpr uintptr_t WeaponManager     = 0xAC61A78; // 180755064
    constexpr uintptr_t GameManager       = 0xAC58C00; // 180718592
    constexpr uintptr_t TouchController   = 0xAC60B48; // 180751176
    constexpr uintptr_t AntiCheatManager  = 0xAC4DA30;
    constexpr uintptr_t GunController     = 0xAC59040; // from Halalium profile / generated
    constexpr uintptr_t PlayerMainCamera  = 0xAC5E188;
}

// ---------------------------------------------------------------------------
// PlayerManager instance resolution + fields
// Community path: +0x90 -> +0x10 -> +0x0  (matches static_fields@0x90)
// ---------------------------------------------------------------------------
namespace PlayerManager {
    constexpr uintptr_t static_fields = Il2Cpp::klass_static_fields; // 0x90
    constexpr uintptr_t static_slot   = 0x10; // common instance slot
    constexpr uintptr_t static_alt    = 0x0;

    constexpr uintptr_t players_list  = 0x28; // List/dict of players
    constexpr uintptr_t local_player  = 0x70; // local PlayerController*
    // dump also has another PlayerController* @ 0x68
    constexpr uintptr_t alt_player    = 0x68;
}

namespace List {
    constexpr uintptr_t buffer = 0x18;
    constexpr uintptr_t size   = 0x20;
    constexpr uintptr_t entry  = 0x30; // first element bias
    constexpr uintptr_t stride = 0x18;
}

// ---------------------------------------------------------------------------
// PlayerController fields (dump TypeDef + Halalium LDR confirmation)
// Confirmed in libhalalium: +0x160 PhotonPlayer, +0x88 Weaponry, +0xA0 weapon
// ---------------------------------------------------------------------------
namespace Player {
    constexpr uintptr_t main_camera_holder   = 0x28;
    constexpr uintptr_t character_view       = 0x48; // PlayerCharacterView (visibility/biped)
    constexpr uintptr_t character_view_alt   = 0x50;
    constexpr uintptr_t team                 = 0x79; // byte
    constexpr uintptr_t aim_controller       = 0x80;
    constexpr uintptr_t weaponry_controller  = 0x88;
    constexpr uintptr_t mecanim_controller   = 0x90;
    constexpr uintptr_t movement_controller  = 0x98;
    constexpr uintptr_t arms_animation       = 0xA0;
    constexpr uintptr_t hit_controller       = 0xA8;
    constexpr uintptr_t material_controller  = 0xB0; // NOT occlusion
    constexpr uintptr_t occlusion_controller = 0xB8; // use this for vis checks
    constexpr uintptr_t network_controller   = 0xC0;
    constexpr uintptr_t sound_controller     = 0xE0;
    constexpr uintptr_t main_camera          = 0xE8; // PlayerMainCamera
    constexpr uintptr_t character_controller = 0x118;
    constexpr uintptr_t photon_view          = 0x150;
    constexpr uintptr_t photon_player        = 0x160;
    constexpr uintptr_t character_visible    = 0xD8; // Halalium Update strb
}

namespace Weaponry {
    constexpr uintptr_t weapon_controller = 0xA0; // current WeaponController*
    constexpr uintptr_t kit_controller    = 0x98;
}

namespace Weapon {
    constexpr uintptr_t parameters     = 0xA8;
    constexpr uintptr_t id             = 0x18; // via parameters/name path variants
    constexpr uintptr_t gun_parameters = 0x160;
}

namespace Aim {
    constexpr uintptr_t aiming_data         = 0x90;
    constexpr uintptr_t aim_pitch           = 0x18;
    constexpr uintptr_t aim_yaw             = 0x1C;
    constexpr uintptr_t aim_pitch_target    = 0x24;
    constexpr uintptr_t aim_yaw_target      = 0x28;
    constexpr uintptr_t aim_camera_transform= 0xC0;
}

namespace Movement {
    constexpr uintptr_t translation_data = 0xB0;
}

namespace Transform {
    constexpr uintptr_t data     = 0xB0;
    constexpr uintptr_t position = 0x44;
}

namespace Camera {
    constexpr uintptr_t transform = 0x20;
    constexpr uintptr_t ptr       = 0x10;
    // view matrix: community lists 0xF0; Halalium/wintex also use 0x100 in places — verify in-game
    constexpr uintptr_t matrix    = 0xF0;
}

namespace Photon {
    constexpr uintptr_t name             = 0x20;
    // Halalium Update ldrb [photon,#0x30] — isLocal (pack layout)
    constexpr uintptr_t is_local         = 0x30;
    constexpr uintptr_t properties       = 0x38;
    constexpr uintptr_t properties_count = 0x20;
    constexpr uintptr_t properties_list  = 0x18;
    constexpr uintptr_t prop_key         = 0x28;
    constexpr uintptr_t prop_value       = 0x30;
    constexpr uintptr_t prop_stride      = 0x18;
    constexpr uintptr_t value_data       = 0x10;
}

// GameController (dump) — PlayerControls moved to 0x2B0 on this build
namespace GameController {
    constexpr uintptr_t static_instance_field = 0x8; // backing field on statics
    constexpr uintptr_t player_controls       = 0x2B0; // was 0x2A0 on older builds
    constexpr uintptr_t local_player          = 0x2C0;
}

namespace PlayerControls {
    constexpr uintptr_t touch_controller = 0x38;
}

namespace Skeleton {
    constexpr uintptr_t biped_map       = 0x48; // under character_view
    constexpr uintptr_t head            = 0x20;
    constexpr uintptr_t neck            = 0x28;
    constexpr uintptr_t spine           = 0x30;
    constexpr uintptr_t spine1          = 0x38;
    constexpr uintptr_t spine2          = 0x40;
    constexpr uintptr_t left_shoulder   = 0x48;
    constexpr uintptr_t left_upperarm   = 0x50;
    constexpr uintptr_t left_forearm    = 0x58;
    constexpr uintptr_t left_hand       = 0x60;
    constexpr uintptr_t right_shoulder  = 0x68;
    constexpr uintptr_t right_upperarm  = 0x70;
    constexpr uintptr_t right_forearm   = 0x78;
    constexpr uintptr_t right_hand      = 0x80;
    constexpr uintptr_t hip             = 0x88;
    constexpr uintptr_t left_upleg      = 0x90;
    constexpr uintptr_t left_leg        = 0x98;
    constexpr uintptr_t left_foot       = 0xA0;
    constexpr uintptr_t left_toebase    = 0xA8;
    constexpr uintptr_t right_upleg     = 0xB0;
    constexpr uintptr_t right_leg       = 0xB8;
    constexpr uintptr_t right_foot      = 0xC0;
    constexpr uintptr_t right_toebase   = 0xC8;
    constexpr uintptr_t transform_object= 0x10;
    constexpr uintptr_t matrix_ptr      = 0x28;
    constexpr uintptr_t index           = 0x30;
    constexpr uintptr_t matrix_list     = 0x18;
    constexpr uintptr_t matrix_indices  = 0x20;
}

// Occlusion / visibility (PlayerOcclusionController @ Player+0xB8)
namespace Occlusion {
    constexpr uintptr_t vis_state = 0x34; // == 2 visible (common pattern)
    constexpr uintptr_t occluded  = 0x38; // != 1
}

// ---------------------------------------------------------------------------
// il2cpp API RVAs (api_0.39.2.txt) — optional for internal name resolve
// ---------------------------------------------------------------------------
namespace Api {
    constexpr uintptr_t il2cpp_class_from_name          = 0x9D5B510;
    constexpr uintptr_t il2cpp_class_get_method_from_name = 0x9D5959C;
    constexpr uintptr_t il2cpp_runtime_invoke           = 0x5C86F28;
    constexpr uintptr_t il2cpp_domain_get               = 0x5C86D54;
    constexpr uintptr_t il2cpp_string_new               = 0x5C86F3C;
    constexpr uintptr_t globalMetadata                  = 0xB04AC00;
}

} // namespace Offsets0392
