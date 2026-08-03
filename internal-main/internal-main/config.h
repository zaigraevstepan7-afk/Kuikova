#pragma once
#include "globals.hpp"
#include "imgui.h"

#include "includes/oxorany/Oxorany.hpp"
#include <dirent.h>
#include <vector>

namespace configs
{
    bool Input(const char *text, char *buf, bool *isOskOpened);
    void cfgList(std::vector<std::string> &vector);
    void cfgRead(const char *filename);
    void cfgWrite(const char *filename);
}
