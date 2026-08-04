#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <android/log.h>

// Halalium-style: resolve game image base from /proc/self/maps (r-xp).
// Prefer libil2cpp.so (TypeInfo + ScriptMethod RVAs). Fall back to libunity.so
// because Halalium's own finder string is "libunity.so" on some SO2 builds.
inline uintptr_t find_module_base_rx(const char *lib_name)
{
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return 0;
    char line[512];
    uintptr_t best = 0;
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t start = 0, end = 0;
        char perms[8]{}, path[256]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s %*s %*s %*s %255s", &start, &end, perms, path) < 3)
            continue;
        if (!strstr(path, lib_name))
            continue;
        if (perms[2] != 'x')
            continue;
        best = start;
        break;
    }
    fclose(f);
    return best;
}

inline uintptr_t resolve_il2cpp_base()
{
    uintptr_t b = find_module_base_rx("libil2cpp.so");
    if (b)
    {
        __android_log_print(ANDROID_LOG_INFO, "kikaium", "game base libil2cpp.so=%p", (void *)b);
        return b;
    }
    b = find_module_base_rx("libunity.so");
    if (b)
        __android_log_print(ANDROID_LOG_INFO, "kikaium", "game base libunity.so=%p (Halalium path)", (void *)b);
    else
        __android_log_print(ANDROID_LOG_ERROR, "kikaium", "game base NOT FOUND");
    return b;
}

inline bool maps_contains_exec(uintptr_t addr)
{
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return false;
    char line[512];
    bool ok = false;
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t s = 0, e = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &s, &e, perms) < 3)
            continue;
        if (addr >= s && addr < e && perms[2] == 'x')
        {
            ok = true;
            break;
        }
    }
    fclose(f);
    return ok;
}
