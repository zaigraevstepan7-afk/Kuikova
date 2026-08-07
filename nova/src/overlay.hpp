#pragma once
#include "game.hpp"

struct NovaConfig {
    bool show_menu = true;
    bool esp_box = true;
    bool esp_name = true;
    bool esp_snapline = true;
    bool esp_team_check = false; // off by default — wrong team byte must not hide ESP
    float box_thickness = 2.0f;
};

NovaConfig& nova_cfg();
void nova_overlay_frame(int width, int height, GameState& st);
void nova_overlay_shutdown();
bool nova_overlay_ensure_imgui();
void nova_feed_touch(float x, float y, bool down);
void nova_feed_touch_norm(float nx, float ny, bool down); // 0..1
void nova_set_hook_mode(int mode);
