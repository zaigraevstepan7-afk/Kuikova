#pragma once
#include "globals.hpp"

class misc {
    public:
    void init(c_player_controller *player);
};
inline misc *c_misc = new misc();