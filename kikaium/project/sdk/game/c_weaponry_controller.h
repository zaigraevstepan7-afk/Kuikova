#pragma once

class c_weapon_controller;
class c_player_controller;
#pragma pack(1)
class c_weaponry_controller
{
    char paddd[0x58];

public:
    void *m_pWeaponsByByte;
    void *m_pWeaponsList;
    void *m_pBytesList;
    void *m_pItemsList;
    c_player_controller *m_pPlayerController;
    void *m_pMecanimController;
    uint8_t m_iCurrentByte;
    bool m_bIsActive;
    char pad_0x8A[0x6];
    void *m_pWeaponPickupCtrl;
    void *m_pKitController;
    c_weapon_controller *m_pCurrentWeapon;
    bool m_bFlag1;
    bool m_bFlag2;
    char pad_0xAA[0x2];
    float m_fValue1;
    float m_fValue2;
    char pad_0xB4[0x4];
    uintptr_t m_pWeaponManager;
};
#pragma pack()