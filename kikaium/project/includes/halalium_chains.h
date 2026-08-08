#pragma once
// Halalium-style field chains + Safe writes (Update_Halalium_Hooks / SkinChanger).
#include "includes/halalium_mem.h"
#include "includes/structs.h"
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
    // Halalium: ldr [player,#0x88]; ldr [x,#0xa0]
    void *w = weaponry(player);
    return w ? hmem::field_ptr(w, Offsets::Weaponry::weapon_controller) : nullptr;
}

inline void *gun_params(void *weapon)
{
    // Halalium: ldr [weapon, #0x168]
    return weapon ? hmem::field_ptr(weapon, 0x168) : nullptr;
}

inline void *weapon_params_alt(void *weapon)
{
    // WeaponController.parameters @0xA8 (SkinChanger / dump)
    return weapon ? hmem::field_ptr(weapon, 0xA8) : nullptr;
}

inline void *main_camera(void *player)
{
    // LDR [player, #0xE8]; cbz
    return hmem::field_ptr(player, Offsets::Player::main_camera);
}

// Halalium Update @0x1d7d80: ldr [pmc,#0x28]; cbz; ldr [x,#0x30]; cbz → Unity Camera*
inline void *unity_camera_from_pmc(void *pmc)
{
    if (!pmc)
        return nullptr;
    void *nested = hmem::field_ptr(pmc, Offsets::PlayerMainCamera::nested); // +0x28
    if (!nested)
        return nullptr;
    return hmem::field_ptr(nested, Offsets::PlayerMainCamera::unity_camera); // +0x30
}

inline void *unity_camera(void *player)
{
    // Player+0xE8 → +0x28 → +0x30 (in-process LDR + null)
    return unity_camera_from_pmc(main_camera(player));
}

inline void *movement(void *player)
{
    return hmem::field_ptr(player, Offsets::Player::movement_controller);
}

inline void *game_controls(void *game)
{
    return hmem::field_ptr(game, Offsets::GameController::player_controls);
}

inline int player_id(void *player)
{
    return hmem::field<int>(player, 0x158, 0);
}

inline uint8_t inventory_id(void *weapon)
{
    void *params = weapon_params_alt(weapon);
    return params ? hmem::field<uint8_t>(params, 0x18, 0) : 0;
}

inline void set_visible(void *player, bool on = true)
{
    // Halalium Update: strb #1 → [player,#0xd8]
    hmem::set_field<uint8_t>(player, Offsets::Player::character_visible, on ? 1 : 0);
}

inline void safe_set_int(void *obj, uintptr_t off, int value)
{
    if (!obj)
        return;
    uintptr_t addr = reinterpret_cast<uintptr_t>(obj) + off;
    if (!hmem::writable(addr, sizeof(safe_t<int>)))
        return;
    reinterpret_cast<safe_t<int> *>(addr)->set(value);
}

inline void safe_set_float(void *obj, uintptr_t off, float value)
{
    if (!obj)
        return;
    uintptr_t addr = reinterpret_cast<uintptr_t>(obj) + off;
    if (!hmem::writable(addr, sizeof(safe_t<float>)))
        return;
    reinterpret_cast<safe_t<float> *>(addr)->set(value);
}

inline void safe_set_bool(void *obj, uintptr_t off, bool value)
{
    if (!obj)
        return;
    uintptr_t addr = reinterpret_cast<uintptr_t>(obj) + off;
    if (!hmem::writable(addr, sizeof(safe_t<bool>)))
        return;
    reinterpret_cast<safe_t<bool> *>(addr)->set(value);
}

// Halalium Inf Ammo
inline void apply_inf_ammo(void *weapon)
{
    if (!weapon)
        return;
    safe_set_int(weapon, 0x120, 0x45);
    safe_set_int(weapon, 0x128, 0x45);
    safe_set_bool(weapon, 0x130, true);
    safe_set_bool(weapon, 0x138, true);
}

// Halalium Fire Rate → TimeFired @0x108 (+ interval crush)
inline void apply_fire_rate(void *weapon)
{
    if (!weapon)
        return;
    safe_set_float(weapon, 0x108, 0.f);
    safe_set_float(weapon, 0x110, 0.02f);
}

// Halalium Wallshot Safe writes on GunParameters*
inline void apply_wallshot(void *weapon)
{
    void *params = gun_params(weapon);
    if (!params)
        return;
    safe_set_float(params, 0x2DC, 9999.f);
    safe_set_int(params, 0x264, 0x1869F);
    safe_set_float(params, 0x258, 9999.f);
}

} // namespace hchain
