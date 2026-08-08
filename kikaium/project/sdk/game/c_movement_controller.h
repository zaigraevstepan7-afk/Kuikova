#pragma once

class c_player_controller;
class c_player_translation_parameters;
class c_translation_data;
class c_movement_cmd;
#pragma pack(1)
class c_movement_controller
{
    char paddddd[0x58];

public:
    c_player_controller *m_pPlayerController;
    void *m_pOcclusionController;
    bool m_bNeverIdle;
    char pad_0x69[0x7];
    void *m_pTransform;
    c_movement_cmd *m_pMovementCMD;
    float m_fValue1;
    float m_fValue2;
    void *m_pCharacterController;
    void *m_pTrigger;
    void *m_pDataArray;
    void *a;
    c_player_translation_parameters *m_pTranslationParams;
    c_translation_data *m_pTranslationData;
    void *m_pMovementState2;
    void *m_pCharacterTransform;
    void *m_pMecanimController;
    float m_fTimestamp;
    char padon[0x4];
    void *m_pEventList1;
    void *m_pEventList2;
    void *m_pEventArray;
};
#pragma pack()

#pragma pack(push, 1)
class c_movement_cmd
{
    char __pad0[0x10];

public:
    Vector3 m_vecTemp;
    float m_fSideMove;
    float m_fForwardMove;
    bool m_bCrouch;
    bool m_bJump;
};
#pragma pack(pop)
#pragma pack(1)
class c_jump_parameters {
    char pad[0x60];
    public:
    float jump_speed;
};
#pragma pack()
#pragma pack(1)
class c_walk_parameters {
    public: 
    char pad[0x10];
    float m_fSpeed;
};
#pragma pack()
#pragma pack(1)
class c_player_translation_parameters
{
    char pad[0x10];

public:
    float m_fSpeedDefault; 
    char pad34[0x18];                        
    float m_fMoveDirectionChangeSpeed;    
    float m_fMecanimDirectionChangeSpeed; 
    char pad3fg4[0x4];                      
    c_walk_parameters *m_pWalkParameters;              
    void *m_pIdleParameters;              
    void *m_pCrouchParameters;            
    c_jump_parameters *m_pJumpParameters;              
    void *m_pColliderParameters;          
    void *m_pGeneralCurveTypes;           
};
#pragma pack()

#pragma pack(1)
class c_translation_data
{
    char pad[0x68];
    public:
    Vector3 a;
};
#pragma pack()