#pragma once
#include "globals.hpp"

using namespace structs;
#pragma pack(1)
class c_photon_player
{
    char padddd[0x18];

public:
    int m_iActorID; // 0x18
    char padd[0x4];
    monoString *m_nameField; // 0x20
    monoString *m_UserId;    // 0x28 (pack(1) sequential)
    // Halalium Update @0x1d7a5c: ldrb [photon,#0x30] — isLocal (NOT Melodium comment 0x28)
    bool m_bIsLocal;        // 0x30
    bool m_bIsInactive;     // 0x31
    char paddddddd[0x6];
    monoDictionary<monoString *, void *> *m_pCustomProperties; // 0x38
    uintptr_t m_pTagObject;                                    // 0x40

    void *player_prop(const char *key);
    int get_health();
};
#pragma pack()

//static_assert(offsetof(c_photon_player, m_nameField) == 0x20);