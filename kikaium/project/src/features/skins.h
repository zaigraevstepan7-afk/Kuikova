#pragma once
#include "globals.hpp"

// Halalium SkinChanger @0x1d9e00 (called from Update @0x1d7dc0)
class skins
{
public:
    void tick(c_player_controller *local);
};

inline skins *c_skins = new skins();
