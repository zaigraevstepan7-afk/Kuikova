#pragma once

#include <cstdint>

struct ChamsConfig {
    bool enabled = true;
    bool team_check = true;
    bool local_chams = false;
    int  material = 1; // 0 Diffuse, 1 Internal-Colored, 2 Standard glass, 3 Standard metal, 4 Transparent
    float color[4] = {1.f, 0.15f, 0.15f, 1.f};
};

ChamsConfig& chams_cfg();

// Resolve Unity/il2cpp APIs + hook PlayerController.Update (libunity).
bool chams_install();

// Call after menu edits color/material so cached mat rebuilds once.
void chams_bump_cfg();

int  chams_applied();
int  chams_hook_ok(); // 0 none, 1 a64, 2 dobby
const char* chams_status();
