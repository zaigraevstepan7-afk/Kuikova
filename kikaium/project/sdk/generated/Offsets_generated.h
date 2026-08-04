// AUTO-GENERATED / Halalium-only map — Source SO: libhalalium.so
// BuildID: 12532fca99debbaa836dbbea6e5cceec95f5bbbb
// Version: t.me/lemminghack, 0.39.2
// Product: Kikaium — ONLY offsets proven from Halalium RE (profile + decompile).
// Do NOT add Melodium / community Unity method RVAs here.
#pragma once
#include <cstdint>

namespace OffsetsGenerated {

namespace Il2Cpp {
    constexpr uintptr_t klass_static_fields = 0x90; // profile + api layout
}

namespace TypeInfo {
    // libil2cpp RVAs — halalium_profile.json typeinfo
    constexpr uintptr_t PlayerManager      = 0xAC5E190;
    constexpr uintptr_t GameController     = 0xAC58BB0;
    constexpr uintptr_t PhotonNetwork      = 0xAC5DE18;
    constexpr uintptr_t BombManager        = 0xAC4FAC0;
    constexpr uintptr_t InventoryManager   = 0xAC5C018;
    constexpr uintptr_t PlayerControls     = 0xAC5E0E0;
    constexpr uintptr_t PlayerController   = 0xAC5E0D8;
    constexpr uintptr_t WeaponController   = 0xAC61A18;
    constexpr uintptr_t WeaponManager      = 0xAC61A78;
    constexpr uintptr_t GameManager        = 0xAC58C00;
    constexpr uintptr_t TouchController    = 0xAC60B48;
    constexpr uintptr_t AntiCheatManager   = 0xAC4DA30;
    constexpr uintptr_t GunController      = 0xAC59040;
    constexpr uintptr_t PlayerMainCamera   = 0xAC5E188;
}

namespace PlayerManager {
    constexpr uintptr_t players_list = 0x28;
    constexpr uintptr_t local_player = 0x70;
    constexpr uintptr_t alt_player   = 0x68;
}

namespace Player {
    // profile.fields + Update_Halalium_Hooks LDR/STR
    constexpr uintptr_t photon_player        = 0x160;
    constexpr uintptr_t weaponry_controller  = 0x88;
    constexpr uintptr_t occlusion_controller = 0xB8;
    constexpr uintptr_t main_camera          = 0xE8;
    constexpr uintptr_t team                 = 0x79;
    constexpr uintptr_t aim_controller       = 0x80;
    constexpr uintptr_t hit_controller       = 0xA8;
    constexpr uintptr_t movement_controller  = 0x98;
    constexpr uintptr_t character_view       = 0x48;
    constexpr uintptr_t photon_view          = 0x150;
    constexpr uintptr_t character_visible    = 0xD8; // Update strb
}

namespace Weaponry {
    constexpr uintptr_t weapon_controller = 0xA0;
}

namespace GameController {
    constexpr uintptr_t player_controls = 0x2B0;
    constexpr uintptr_t local_player    = 0x2C0;
}

namespace Camera {
    constexpr uintptr_t matrix = 0xF0; // profile only — no transform/ptr in Halalium
}

namespace Photon {
    constexpr uintptr_t is_local = 0x30; // Update ldrb [photon,#0x30]
}

namespace Method {
    // libunity RVAs — egl_install / profile / SkinChanger / Bypass
    constexpr uintptr_t PlayerController_Update     = 0x8E7C40C;
    constexpr uintptr_t PlayerController_LateUpdate = 0x8E7CF50; // Update+0xB44
    constexpr uintptr_t HookSecondary               = 0x8E0085C;
    constexpr uintptr_t HookTertiary                = 0x79FE5E0;
    constexpr uintptr_t HookTertiaryAlt             = 0x147E970;
    constexpr uintptr_t HookExtraA                  = 0x8D663EC;
    constexpr uintptr_t HookExtraB                  = 0x8D2B2B0;
    constexpr uintptr_t AntiCheat_OnStart_getrr     = 0x8B9579C;
    constexpr uintptr_t Gun_ExecuteCommands         = 0x0;
    constexpr uintptr_t Hit_StrictHit               = 0x0;
    constexpr uintptr_t Skin_CreateOrGet            = 0x8E8FE50;
    constexpr uintptr_t Skin_Mid                    = 0x8E852D4;
    constexpr uintptr_t Skin_Equip                  = 0x8E7F7F4;
}

// Unity engine method RVAs embedded in Halalium callbacks (same module as Method::*)
namespace UnityMethod {
    constexpr uintptr_t set_fov         = 0x5FB7BC0; // Update_Halalium_Hooks
    constexpr uintptr_t set_active      = 0x6009294; // secondary_hook_cb
    constexpr uintptr_t get_game_object = 0x5FFAEFC; // secondary_hook_cb
    constexpr uintptr_t Update_aux_A    = 0x8C69CB0; // Update_Halalium_Hooks
    constexpr uintptr_t Update_aux_B    = 0x8E8AE24; // Update_Halalium_Hooks
    constexpr uintptr_t LateUpdate_A    = 0x8C7BE04; // LateUpdate_cb
    constexpr uintptr_t LateUpdate_B    = 0x5CAAFC4; // LateUpdate_cb
    constexpr uintptr_t LateUpdate_C    = 0x5DEADA4; // LateUpdate_cb
}

namespace Hook {
    constexpr bool use_egl_swap_buffers = true;
    constexpr bool open_menu_via_watermark_click = true;
    // Halalium: no GameController VMT. PC Update/Late VMT only if tracked RVA fails
    constexpr bool use_vmt_update_hooks = true;
    // Halalium Bypass_getrr — destroy tracked hooks during AntiCheat OnStart
    constexpr bool use_getrr_bypass = true;
    // Halalium always installs Secondary/Tertiary/ExtraA/B as TRACKED
    constexpr bool use_secondary_hooks = true;
    // Halalium path: InputConsumer::consume (UNTRACKED like egl)
    constexpr bool use_input_consume = true;
}

} // namespace OffsetsGenerated
