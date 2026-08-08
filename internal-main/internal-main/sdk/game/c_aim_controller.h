#pragma once

class c_aiming_data;
#pragma pack(1)
class c_aim_controller
{
    char pad[0x53];

public:
    bool m_bOverrideSpineRotation;
    uint8_t m_visState;
    char _pad3x[3];
    float m_fSensitivityX;
    float m_fSensitivityY;
    float m_fMinimumX;
    float m_fMaximumX;
    void *m_pFPSP_go;
    void *m_pSpineDirector;
    void *m_pFPSCamera;
    void *m_pCamTransform;
    void *m_pAimingParameters;
    c_aiming_data *m_pAimingData;
    void *m_pInterpolatorsBunch;
    void *m_pTuningParams;
    float m_fHeadDampingSpeed;
    char _padx4[4];
    void *m_pPlayerController;
    void *m_pMovementController;
    void *m_pSomeTransform;
    void *m_pMecanimController;
    void *m_pBEBAABB;
    bool m_bGFBGDCFDHCDFBGD;
    char _pad1[0x7];
    void *m_pDBGDCFCHHHAACGE;
    void *m_view;
    Vector3 m_v3;
    char _pad2[0x4];
    void *m_pState2;
    void *m_MoveState;
    void *m_pTransformTR1;
    void *m_pTransformTR2;
    char _pose0[0x1C];
    char _pose1[0x1C];
    char _pose2[0x1C];
    char _pose3[0x1C];
    char _pose4[0x20];
    void *WeaponAnimationParameters;
    void *m_pFBFFFHBDHADCECC;
    void *m_pAction;
    bool m_bDEDADHGACADDAED;
    char _pad3[0x3];
    float m_fDAFFABGECGFDDCC;
    float m_fFEACBFFDFFAAGCH;
    char padding[0x4];
    void *m_pHBFDEEBEEGBFAHE;
    float m_fGAGAFEGDAGEFHAD;
    float m_fADEFCFDAFEGEGGC;
    void *m_arrayxyiznaet;
    void *m_pWeaponryController;
    char _quat[0x10];
    bool ToolPivotTuningEnabled;
    bool SpineRotationEnabled;
    char _pad4[0x2];
    Vector3 m_v3b;
    Vector3 m_v3c;
    Vector3 m_v3d;
    char _poseEnd[0x30];
};
#pragma pack()

#pragma pack(1)
class c_aiming_data
{
    char pad[0x10];
public:
    float m_fAimProgress;
    float m_fZoomFactor;
    euler_angles_t cur_aim_ang; // 0x18
    euler_angles_t cur_euler_ang; // 0x24
    void *m_WeaponTransform;
    float m_fStability;
    float m_fSwayFactor;
};
#pragma pack()
