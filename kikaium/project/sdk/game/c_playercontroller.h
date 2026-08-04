#pragma once
#include <cstdint>

enum class team_t : uint8_t
{
    none = 0,
    tr = 1,
    ct = 2,
    spectator = 3,
};

enum class view_mode_t : uint8_t
{
    none = 0,
    fps = 1,
    tps = 2,
};
class c_transform;
#pragma pack(1)
class c_biped_map
{
    char pad[0x20];

public:
    c_transform *head;
    c_transform *neck;
    c_transform *spine;
    c_transform *spine1;
    c_transform *spine2;
    c_transform *left_shoulder;
    c_transform *left_upperarm;
    c_transform *left_forearm;
    c_transform *left_hand;
    c_transform *right_shoulder;
    c_transform *right_upperarm;
    c_transform *right_forearm;
    c_transform *right_hand;
    c_transform *hip;
    c_transform *left_up_leg;
    c_transform *left_leg;
    c_transform *left_foot;
    c_transform *left_toe_base;
    c_transform *right_up_leg;
    c_transform *right_leg;
    c_transform *right_foot;
    c_transform *right_toe_base;
    c_transform *left_hand_index1;
    c_transform *left_hand_index2;
    c_transform *left_hand_index3;
    c_transform *left_hand_index4;
    c_transform *left_in_hand_middle;
    c_transform *left_hand_middle1;
    c_transform *left_hand_middle2;
    c_transform *left_hand_middle3;
    c_transform *left_hand_middle4;
    c_transform *left_in_hand_pinky;
    c_transform *left_hand_pinky1;
    c_transform *left_hand_pinky2;
    c_transform *left_hand_pinky3;
    c_transform *left_hand_pinky4;
    c_transform *left_in_hand_ring;
    c_transform *left_hand_ring1;
    c_transform *left_hand_ring2;
    c_transform *left_hand_ring3;
    c_transform *left_hand_ring4;
    c_transform *left_in_hand_thumb;
    c_transform *left_hand_thumb1;
    c_transform *left_hand_thumb2;
    c_transform *left_hand_thumb3;
    c_transform *right_in_hand_index;
    c_transform *right_hand_index1;
    c_transform *right_hand_index2;
    c_transform *right_hand_index3;
    c_transform *right_hand_index4;
    c_transform *right_in_hand_middle;
    c_transform *right_hand_middle1;
    c_transform *right_hand_middle2;
    c_transform *right_hand_middle3;
    c_transform *right_hand_middle4;
    c_transform *right_in_hand_pinky;
    c_transform *right_hand_pinky1;
    c_transform *right_hand_pinky2;
    c_transform *right_hand_pinky3;
    c_transform *right_hand_pinky4;
    c_transform *right_in_hand_ring;
    c_transform *right_hand_ring1;
    c_transform *right_hand_ring2;
    c_transform *right_hand_ring3;
    c_transform *right_hand_ring4;
    c_transform *right_in_hand_thumb;
    c_transform *right_hand_thumb1;
    c_transform *right_hand_thumb2;
    c_transform *right_hand_thumb3;
};
#pragma pack()
class c_transform;
class c_object_occludee;
#pragma pack(1)
class c_player_character_view
{
    char pad[0x30];

public:
    bool oclussion; // 0x30
    char padd[0x17];
    c_biped_map *c_biped; // 0x48
};
#pragma pack()
class c_player_main_camera;
class c_photon_player;
class c_aim_controller;
class c_weaponry_controller;
class c_movement_controller;
class c_characher_lod_group;
class c_renderer;
#pragma pack(1)
class c_arms_lod_group
{
    char pad[0x28];

public:
    c_renderer *arms;   // 0x28 _armsMeshRenderer
    c_renderer *gloves; // 0x30 _glovesMeshRenderer
};
#pragma pack()
#pragma pack(1)
class c_characher_controller
{
public:
    Vector3 get_velocity()
    {
        if (!this || !c_fn || !c_fn->get_velocity)
            return Vector3{};
        return c_fn->get_velocity(this);
    }
};
#pragma pack()
// PlayerController layout for Standoff 2 0.39.2 (okaakka / feng dump)
#pragma pack(1)
class c_player_controller
{
    char __pad[0x28];

public:
    c_transform *m_pMainCameraHolder;              // 0x28
    c_transform *m_pFPSCameraHolder;               // 0x30
    void *m_pFPSDirective;                         // 0x38
    void *m_pZones;                                // 0x40
    c_player_character_view *m_pCharacterView;     // 0x48
    c_player_character_view *m_pCharacterViewAlt;  // 0x50
    void *m_pObj58;                                // 0x58
    void *m_pObj60;                                // 0x60
    void *m_pObj68;                                // 0x68
    void *m_pObj70;                                // 0x70
    bool m_bReconnectedInstance;                   // 0x78
    team_t m_team;                                 // 0x79
    char __pad0[0x2];                              // 0x7A
    float m_fLocalTime;                            // 0x7C
    c_aim_controller *m_pAim;                      // 0x80
    c_weaponry_controller *m_pWeaponry;            // 0x88
    void *m_pMecanim;                              // 0x90
    c_movement_controller *m_pMovement;            // 0x98
    void *m_pArmsAnimation;                        // 0xA0
    void *m_pHit;                                  // 0xA8
    void *m_pMaterial;                             // 0xB0 PlayerMaterialController
    void *m_pOcclusion;                            // 0xB8 PlayerOcclusionController
    void *m_pNetwork;                              // 0xC0
    void *m_pArmsLOD;                              // 0xC8 ArmsLodGroup (alt path)
    void *m_pCharacter;                            // 0xD0
    bool m_bCharacterVisible;                      // 0xD8
    bool m_bool0;                                  // 0xD9
    char __pad1[0x2];                              // 0xDA
    float m_fSnapshotSetTime;                      // 0xDC
    void *m_pSound;                                // 0xE0
    c_player_main_camera *m_pMainCamera;           // 0xE8
    void *m_pFPSCamera;                            // 0xF0
    void *m_pMarker;                               // 0xF8
    c_transform *m_pTransform;                     // 0x100
    void *m_pControllers;                          // 0x108
    void *m_pdControllersByType;                   // 0x110
    c_characher_controller *m_pPhysicCharacter;    // 0x118
    c_arms_lod_group *m_pLOD;                      // 0x120 SkinnedMeshLodGroup/Arms
    c_characher_lod_group *m_pCharacterLOD;        // 0x128
    bool m_bIsPreInitialized;                      // 0x130
    bool m_bIsPostInitialized;                     // 0x131
    char __pad2[0x2];                              // 0x132
    view_mode_t m_viewMode;                        // 0x134
    char __pad3[0x3];                              // 0x135
    void *m_pObj138;                               // 0x138
    void *m_pObj140;                               // 0x140
    int32_t m_iPad144;                             // 0x144
    char __pad4[0x4];                              // 0x148
    void *m_pPhotonView;                           // 0x150
    int32_t m_iID;                                 // 0x158
    int32_t m_iViewID;                             // 0x15C
    c_photon_player *m_pPhoton;                    // 0x160

    void set_tps()
    {
        if (c_fn && c_fn->set_tps)
            c_fn->set_tps(this);
    }
    void set_fps()
    {
        if (c_fn && c_fn->set_fps)
            c_fn->set_fps(this);
    }

    void set_visible()
    {
        // Halalium Through Walls: field character_visible @0xD8 (+ optional MethodInfo)
        m_bCharacterVisible = true;
        if (c_fn && c_fn->set_visible)
            c_fn->set_visible(this);
    }
};
#pragma pack()

#pragma pack(1)
class c_characher_lod_group
{
public:
    char pad[0x30];
    c_renderer *skinned_mesh_render; // 0x30 _meshRenderer
};
#pragma pack()
