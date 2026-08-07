#pragma once
#include <cstdint>

struct GameState {
    bool ready = false;
    uintptr_t il2cpp = 0;
    uintptr_t unity = 0;
    uintptr_t manager = 0;
    uintptr_t local = 0;
    int frame = 0;
    const char* status = "boot";
};

bool game_init();
void game_tick(GameState& st);
