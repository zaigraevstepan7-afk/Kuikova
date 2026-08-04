#include "skins.h"

#include "sdk/game/c_weaponry_controller.h"
#include "sdk/game/c_weapon_controller.h"
#include "sdk/game/c_weapon_parameters.h"
#include "sdk/OffsetsBridge.h"

// Halalium SkinChanger @0x1d9e00 Capstone RE (called from Update @0x1d7dc0):
//   flag → player+0x88 weaponry → +0xa0 weapon → +0xa8 params → id@+0x18
//   create/mid/equip RVAs; applied skin cached at weapon+0xe0

namespace {
constexpr uintptr_t kWeaponSkinId = 0xE0;
}

void skins::tick(c_player_controller *local)
{
    if (!g.b_skin_changer || !local || !unity_base)
        return;

    auto *weaponry = local->m_pWeaponry;
    if (!weaponry)
        return;

    auto *weapon = weaponry->m_pCurrentWeapon;
    if (!weapon)
        return;

    auto *params = weapon->m_pParameters;
    if (!params)
        return;

    const uint8_t cur_id = params->m_id;
    if (g.i_skin_weapon <= 0 || cur_id != (uint8_t)g.i_skin_weapon)
        return;

    const int want_skin = g.i_skin_id;
    if (want_skin < 0)
        return;

    int *applied = reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(weapon) + kWeaponSkinId);
    if (*applied == want_skin)
        return;

    using create_t = void *(*)(void *weaponry, int weapon_id, int player_id, int skin_id, int z, void *method);
    using mid_t    = void (*)(void *weaponry, uint8_t cur_byte, void *method);
    using equip_t  = void (*)(void *weaponry, void *new_weapon, int z, void *method);

    auto create = reinterpret_cast<create_t>(unity_base + Offsets::Method::Skin_CreateOrGet);
    auto mid    = reinterpret_cast<mid_t>(unity_base + Offsets::Method::Skin_Mid);
    auto equip  = reinterpret_cast<equip_t>(unity_base + Offsets::Method::Skin_Equip);
    if (!create || !mid || !equip)
        return;

    void *neu = create(weaponry, g.i_skin_weapon, local->m_iID, want_skin, 0, nullptr);
    if (!neu)
        return;

    mid(weaponry, weaponry->m_iCurrentByte, nullptr);
    equip(weaponry, neu, 0, nullptr);

    LOGI("Skin Changer: Swapped to weapon %d (skin %d)", g.i_skin_weapon, want_skin);
}
