#include "skins.h"

#include "sdk/game/c_weaponry_controller.h"
#include "sdk/game/c_weapon_controller.h"
#include "sdk/game/c_weapon_parameters.h"
#include "sdk/OffsetsBridge.h"
#include "includes/halalium_chains.h"
#include "includes/halalium_mem.h"

// Halalium SkinChanger @0x1d9e00:
//   player+0x88 → +0xa0 → +0xa8 → id@+0x18; cache weapon+0xe0; id player+0x158

namespace {
constexpr uintptr_t kWeaponSkinId = 0xE0;
}

void skins::tick(c_player_controller *local)
{
    if (!g.b_skin_changer || !local || !unity_base)
        return;

    void *weaponry = hchain::weaponry(local);
    if (!weaponry)
        return;

    void *weapon = hchain::current_weapon(local);
    if (!weapon)
        return;

    const uint8_t cur_id = hchain::inventory_id(weapon);
    if (g.i_skin_weapon <= 0 || cur_id != (uint8_t)g.i_skin_weapon)
        return;

    const int want_skin = g.i_skin_id;
    if (want_skin < 0)
        return;

    int applied = 0;
    if (!hmem::read(reinterpret_cast<uintptr_t>(weapon) + kWeaponSkinId, applied))
        return;
    if (applied == want_skin)
        return;

    using create_t = void *(*)(void *weaponry, int weapon_id, int player_id, int skin_id, int z, void *method);
    using mid_t    = void (*)(void *weaponry, uint8_t cur_byte, void *method);
    using equip_t  = void (*)(void *weaponry, void *new_weapon, int z, void *method);

    auto create = reinterpret_cast<create_t>(unity_base + Offsets::Method::Skin_CreateOrGet);
    auto mid    = reinterpret_cast<mid_t>(unity_base + Offsets::Method::Skin_Mid);
    auto equip  = reinterpret_cast<equip_t>(unity_base + Offsets::Method::Skin_Equip);
    if (!create || !mid || !equip)
        return;

    const int pid = hchain::player_id(local);
    void *neu = create(weaponry, g.i_skin_weapon, pid, want_skin, 0, nullptr);
    if (!neu)
        return;

    // m_iCurrentByte on weaponry — Halalium passes it to mid(); read via struct if available
    auto *wry = reinterpret_cast<c_weaponry_controller *>(weaponry);
    mid(weaponry, wry ? wry->m_iCurrentByte : 0, nullptr);
    equip(weaponry, neu, 0, nullptr);

    hmem::write(reinterpret_cast<uintptr_t>(weapon) + kWeaponSkinId, want_skin);
    LOGI("Skin Changer: Swapped to weapon %d (skin %d)", g.i_skin_weapon, want_skin);
}
