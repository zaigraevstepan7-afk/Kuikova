#pragma once
#include "globals.hpp"

class chams
{
    public:
    void enemy(c_player_controller *player);
    void hit(c_player_controller *player);
    void local(c_player_controller *player);
    void arms(c_player_controller *player);
    void weapon(c_player_controller *player);

};

inline chams *c_chams = new chams();