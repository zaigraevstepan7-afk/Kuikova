#pragma once
// Halalium-style field chains used by Update_Halalium_Hooks / SkinChanger.
// All R/W goes through hmem (maps-checked LDR/STR) — same model as injected Halalium.
#include "includes/halalium_mem.h"
#include "sdk/OffsetsBridge.h"

namespace hchain {

inline void *photon(void *player)
{
    return hmem::field_ptr(player, Offsets::Player::photon_player);
}

inline bool is_local(void *player)
{
    void *ph = photon(player);
    if (!ph)
        return false;
    return hmem::field<uint8_t>(ph, Offsets::Photon::is_local, 0) != 0;
}

inline uint8_t team(void *player)
{
    return hmem::field<uint8_t>(player, Offsets::Player::team, 0);
}

inline void *weaponry(void *player)
{
    return hmem::field_ptr(player, Offsets::Player::weaponry_controller);
}

inline void *current_weapon(void *player)
{
    void *w = weaponry(player);
    return w ? hmem::field_ptr(w, Offsets::Weaponry::weapon_controller) : nullptr;
}

inline void *gun_params(void *weapon)
{
    // Halalium: ldr x20, [weapon, #0x168]
    return hmem::field_ptr(weapon, 0x168);
}

inline void set_visible(void *player, bool on = true)
{
    // Halalium: strb w10, [player, #0xd8]
    hmem::set_field<uint8_t>(player, Offsets::Player::character_visible, on ? 1 : 0);
}

inline void *main_camera(void *player)
{
    return hmem::field_ptr(player, Offsets::Player::main_camera);
}

inline void *movement(void *player)
{
    return hmem::field_ptr(player, Offsets::Player::movement_controller);
}

} // namespace hchain
