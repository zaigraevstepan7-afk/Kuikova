#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

struct Mat4 {
    float m[16]{};
};

struct PlayerSnap {
    uintptr_t addr = 0;
    Vec3      feet{};
    Vec3      head{};
    int       team = -1;
    bool      is_local = false;
    std::string name;
};

struct GameState {
    bool ready = false;
    uintptr_t il2cpp = 0;
    uintptr_t manager = 0;
    uintptr_t local = 0;
    Mat4 view_proj{};
    bool has_matrix = false;
    std::vector<PlayerSnap> players;
    int frame = 0;
    const char* status = "boot";
};

bool game_init();
void game_tick(GameState& st);
bool world_to_screen(const Mat4& vp, const Vec3& world, float sw, float sh, float& out_x, float& out_y);
