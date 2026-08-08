#pragma once

#pragma pack(1)

enum class team_t : uint8_t;

class c_inventory_parameters
{
    char pad0[0x18];

public:
    uint8_t m_id; // 0x18
    char pad1[0x7];
    void *name;
    int32_t _cost;
    int32_t _movementRate;
    void *weapon_sprite;
    team_t team;
    bool _isAvailableCreateBuyRequest;
    char pad2[0x2];
    void *_idSafe;
    char pad[0x4];
    void *_costSafe;
    char pad3[0x4];
    void *_movementRateSafe;
    char pad4[0x4];
};
#pragma pack()

#pragma pack(1)

class c_weapon_parameters : public c_inventory_parameters
{
public:
    float m_fFixScale;                       // 0x60
    bool m_bDropBoxCollider;                 // 0x64
    char pad_0x65[0x3];                      // 0x65
    char m_pThrowParameters[8];              // 0x68
    bool m_bInspectable;                     // 0x70
    bool m_bForceTakeAfterPickup;            // 0x71
    bool m_bInspectableBonus;                // 0x72
    char pad_0x73[0x5];                      // 0x73
    char m_pHitDelayParameters[8];           // 0x78
    char m_sCustomAnimationName[8];          // 0x80
    char m_pCustomAnimationWeaponId[8];      // 0x88
    bool m_bIsDroppable;                     // 0x90
    char pad_0x91[0x7];                      // 0x91
    uint8_t m_iShootFireBehaviourType;       // 0x98
    char pad_0x99[0x3];                      // 0x99
    uint8_t m_iShootAdditionalBehaviourType; // 0x9C
    char pad_0x9D[0x3];                      // 0x9D
    float m_fShootInShiftSpeedMult;          // 0xA0
    bool m_bDrawStrecherOnGettingReady;      // 0xA4
    bool m_bHasCancelAimButton;              // 0xA5
    uint8_t m_iCancelAimBehaviourType;       // 0xA6
    uint8_t m_iWeaponAmmoIconType;           // 0xA7
    bool m_bShowHitMarker;                   // 0xA8
    char pad_0xA9[0x3];                      // 0xA9
    uint8_t m_iKillType;                     // 0xAC
    char fsd[0x63];
};
#pragma pack(pop)

class Damage;
class CRecoilParameters;
class Ammunition
{
    char pad0[0x10];

public:
    short _magazineCapacity;
};
enum class _sightType : uint8_t
{
    Default = 0,
    collimator_sight = 1,
    sniper_scope = 2,
};
enum class _rifle_type : uint8_t
{
    not_stated = 0,
    bolt_type = 1,
    semi_automatic = 2,
    automatic = 3,
};
enum class bullet_trace_type : uint8_t
{
    bullet_trace_type_pistol = 0,
    bullet_trace_type_heavy_pistol = 1,
    bullet_trace_type_smg = 2,
    bullet_trace_type_rifle = 3,
    bullet_trace_type_sniper = 4,
    bullet_trace_type_submachine = 5,
    bullet_trace_type_snow_gun = 6,
};

enum class reload_type : uint8_t
{
    reload_type_magazine = 0,
    reload_type_single_round = 1,
};

enum class ammunition_type : uint8_t
{
    ammunition_type_cartridge = 0,
    ammunition_type_shotgun_shell = 1,
};

enum class muzzle_type : uint8_t
{
    muzzle_type_single = 0,
    muzzle_type_double = 1,
};

enum class muzzle_flash_type : int8_t
{
    muzzle_flash_type_none = -1,
    muzzle_flash_type_pistol = 0,
    muzzle_flash_type_smg = 1,
    muzzle_flash_type_rifle = 2,
    muzzle_flash_type_sniper = 3,
    muzzle_flash_type_shotgun = 4,
};

enum class surface_type : uint8_t
{
    surface_type_unknown = 0,
    surface_type_glass = 1,
    surface_type_cardboard = 2,
    surface_type_metal_grate = 3,
    surface_type_wood = 4,
    surface_type_plaster = 5,
    surface_type_tile = 6,
    surface_type_metal = 7,
    surface_type_concrete = 8,
    surface_type_brick = 9,
    surface_type_solid_metal = 10,
    surface_type_thin_metal = 11,
    surface_type_thin_wood = 12,
    surface_type_character = 13,
    surface_type_ground = 14,
    surface_type_surface_mark_trigger = 15,
    surface_type_snow_concrete = 16,
    surface_type_snow_metal = 17,
    surface_type_snow_ground = 18,
    surface_type_solid_metal_no_hole = 19,
    surface_type_gravel = 20,
    surface_type_grass = 21,
    surface_type_paper = 22,
    surface_type_water = 23,
    surface_type_glass_waterfall = 24,
    surface_type_wood_solid = 25,
};

#pragma pack(1)
class c_gun_parameters : public c_weapon_parameters
{
public:
    _sightType _sightType;
    char pad111[0x3];
    bool _drawDefaultRecoilLinesNotInAiming;
    _rifle_type _rifleType;
    bullet_trace_type _bulletTraceType;
    reload_type _reloadType;
    ammunition_type _ammunitionType;
    char pad119[0x7];
    void *_shotgunShellParameters;
    muzzle_type _muzzleType;
    bool _combineMeshInIdle;
    char pad12A[0x2];
    float _scopeAimSensitivityMult;
    void *_ammunition;
    bool _doubledAmmoConsumption;
    char pad139[0x7];
    void *_damage;
    float _hitRange;
    int32_t _fireRate;
    int32_t _recoilControl;
    char pad154[0x4];
    void *_recoilParameters;
    void *_accuracyAdditiveCurve;
    float _accuracyMultOnJump;
    char pad16C[0x4];
    void *_additiveNoAimDispertionCurve;
    float _recoilMultOnCrouch;
    float _accuracyMultOnCrouch;
    float _recoilAimMult;
    float _accuracyAimMult;
    float _recoilAimMultOnCrouch;
    float _accuracyAimMultOnCrouch;
    int32_t _accurateRange;
    char pad194[0x4];
    void *_gunSightViewParameters;
    float _armorPenetration;
    int32_t _penetrationPower;
    float _reloadDuration;
    float _tacticalReloadDuration;
    float _roundInsertDuration;
    float _reloadStartOffset;
    float _roundInsertOffset;
    int32_t _visibleCartridgeCount;
    float _magazineInsertTime;
    float _takeDuration;
    float _preReloadTime;
    int32_t _impulse;
    muzzle_flash_type _muzzleFlashType;
};
#pragma pack(pop)

