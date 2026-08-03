#pragma once

enum visibilityState_t : uint8_t
{
    visibilityState_t_just_switched = 0,
    visibilityState_t_waiting_switch_animation = 1,
    visibilityState_t_set_visible = 2,
};

enum handleState_t : uint8_t
{
    handleState_t_primary = 0,
    handleState_t_secondary = 1,
    handleState_t_not_stated = 2,
};

enum class view_mode_t : uint8_t;

class c_player_controller;
class c_weapon_parameters;

// WeaponController — Standoff 2 0.39.2 dump layout (ends @ 0x100 for GunController)
#pragma pack(1)
class c_weapon_controller
{
    char __pad[0x20];

public:
    c_player_controller *m_pPlayer;      // 0x20
    void *m_pMecanim;                    // 0x28
    void *m_pAnimation;                  // 0x30
    void *m_pAnimationParameters;        // 0x38
    visibilityState_t m_uVisibilityState;// 0x40
    char __pad1[0x3];
    view_mode_t m_viewMode;              // 0x44
    char __pad2[0x3];
    void *m_action0;                     // 0x48
    void *m_action1;                     // 0x50
    void *m_action2;                     // 0x58
    void *m_pTransform;                  // 0x60
    void *m_aControllers;               // 0x68
    void *m_qword0;                      // 0x70
    void *m_qword1;                      // 0x78
    float m_fLocalTime;                  // 0x80
    float m_fDeltaTime;                  // 0x84
    void *m_pLOD;                        // 0x88 WeaponLodGroup
    handleState_t m_uHandleState;        // 0x90
    char pad3[0x3];
    uint8_t m_uAnimationID;              // 0x94
    char pad4[0x3];
    void *m_psAnimationName;             // 0x98
    int32_t m_iOwnerID;                  // 0xA0
    int32_t m_int0;                      // 0xA4
    c_weapon_parameters *m_pParameters;   // 0xA8
    void *m_qword2;                      // 0xB0
    void *m_pWeaponMap;                  // 0xB8
    uint8_t m_uSlotIndex;                // 0xC0
    bool m_bIsActive;                    // 0xC1
    char pad5[0x6];
    void *m_pStatrack;                   // 0xC8
    void *m_pMaterial;                   // 0xD0
    void *m_qword3;                      // 0xD8
    void *m_qword4;                      // 0xE0
    void *m_qword5;                      // 0xE8
    void *m_qword6;                      // 0xF0
    void *m_qword7;                      // 0xF8
};
#pragma pack()

enum class shoot_state : uint8_t
{
    Reloading = 0,
    PreReload = 1,
    Switching = 2,
    TryToSwitch = 3,
    Ready = 4
};

enum class gun_state_t : uint8_t
{
    Aiming = 0,
    AimingReload = 1,
    NotAiming = 2,
    StartingAiming = 3,
    FinishingAiming = 4
};

enum class gun_aiming_mode_t : uint8_t
{
    NotStated = 0,
    Stopped = 2,
    LoopShooting = 3
};

class c_bullet_hit_data;
class c_gun_parameters;

// GunController — 0.39.2 dump (fields used by Melodium features)
#pragma pack(1)
class c_gun_controller : public c_weapon_controller
{
    char __pad_gun0[0x8]; // 0x100

public:
    safe_t<float> m_fTimeFiredSafe;      // 0x108
    safe_t<float> m_fFireIntervalSafe;   // 0x110
    int32_t m_iShotID;                   // 0x118
    int32_t m_iAudioSourceIndex;         // 0x11C
    safe_t<int> m_iCapacitySafe;         // 0x120
    safe_t<int> m_iAmmoSafe;             // 0x128
    safe_t<bool> m_bInfinityAmmoSafe;    // 0x130
    safe_t<bool> m_bInfinityMagazinesSafe; // 0x138
    void *m_pSound;                      // 0x140
    shoot_state m_shootState;            // 0x148
    char __pad_gun1[0x7];
    void *m_qword8;                      // 0x150
    void *m_pRecoilControl;              // 0x158 Action
    void *m_qword9;                      // 0x160
    c_gun_parameters *m_pParameters;      // 0x168
    void *m_pSightView;                  // 0x170
    float m_fLastHandledShotTime;        // 0x178
    float m_fLastShotPlayedTime;         // 0x17C
    void *m_pSnapshot;                   // 0x180
    state_simple_t<uint8_t> *m_pState;   // 0x188
    state_simple_t<uint8_t> *m_pAimMode; // 0x190
    char __pad_gun3[0x230 - 0x198];
    monoDictionary<int, monoList<c_bullet_hit_data *> *> *m_pdCharacterHits; // 0x230
};

static_assert(sizeof(c_weapon_controller) == 0x100, "WeaponController size");
static_assert(offsetof(c_gun_controller, m_fFireIntervalSafe) == 0x110, "FireInterval");
static_assert(offsetof(c_gun_controller, m_bInfinityAmmoSafe) == 0x130, "InfinityAmmo");
static_assert(offsetof(c_gun_controller, m_pParameters) == 0x168, "GunParameters");
static_assert(offsetof(c_gun_controller, m_pAimMode) == 0x190, "AimMode");
static_assert(offsetof(c_gun_controller, m_pdCharacterHits) == 0x230, "CharacterHits");

class c_bullet_hit_data
{
    char pad[0x10];

public:
    Vector3 start;
    Vector3 end;
    float unk;
    int m_iDamage;
};
#pragma pack()
