#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <vector>
#include <mutex>
#include <link.h>
#include <dlfcn.h>
#include <android/log.h>

// Cached /proc/self/maps executable ranges — refresh rarely, never per-RVA.
namespace maps_cache {
inline std::mutex &mu()
{
    static std::mutex m;
    return m;
}
inline std::vector<std::pair<uintptr_t, uintptr_t>> &rx()
{
    static std::vector<std::pair<uintptr_t, uintptr_t>> v;
    return v;
}
inline void refresh()
{
    std::vector<std::pair<uintptr_t, uintptr_t>> next;
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return;
    char line[768];
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t s = 0, e = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &s, &e, perms) < 3)
            continue;
        if (perms[2] == 'x')
            next.emplace_back(s, e);
    }
    fclose(f);
    std::lock_guard<std::mutex> lock(mu());
    rx().swap(next);
}
inline bool is_exec(uintptr_t addr)
{
    std::lock_guard<std::mutex> lock(mu());
    if (rx().empty())
    {
        // unlock-less refresh path: release and refresh
    }
    for (auto &r : rx())
    {
        if (addr >= r.first && addr < r.second)
            return true;
    }
    return false;
}
} // namespace maps_cache

inline void maps_cache_ensure()
{
    std::lock_guard<std::mutex> lock(maps_cache::mu());
    if (!maps_cache::rx().empty())
        return;
    // refresh without holding? simplify: refresh inline
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return;
    char line[768];
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t s = 0, e = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &s, &e, perms) < 3)
            continue;
        if (perms[2] == 'x')
            maps_cache::rx().emplace_back(s, e);
    }
    fclose(f);
}

inline bool maps_contains_exec(uintptr_t addr)
{
    maps_cache_ensure();
    return maps_cache::is_exec(addr);
}

inline uintptr_t find_module_base_maps(const char *needle)
{
    if (!needle || !needle[0])
        return 0;
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return 0;
    char line[768];
    uintptr_t best = 0;
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t start = 0, end = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &start, &end, perms) < 3)
            continue;
        char *path = strchr(line, '/');
        if (!path)
            path = line;
        if (!strstr(path, needle))
            continue;
        if (!best || start < best)
            best = start;
    }
    fclose(f);
    return best;
}

inline uintptr_t find_module_base_phdr(const char *needle)
{
    struct Ctx
    {
        const char *name;
        uintptr_t base;
    } ctx{needle, 0};

    dl_iterate_phdr(
        [](struct dl_phdr_info *info, size_t, void *data) -> int {
            auto *c = reinterpret_cast<Ctx *>(data);
            if (!info || !c->name)
                return 0;
            const char *n = info->dlpi_name ? info->dlpi_name : "";
            if (!n[0] || !strstr(n, c->name))
                return 0;
            c->base = static_cast<uintptr_t>(info->dlpi_addr);
            return 1;
        },
        &ctx);
    return ctx.base;
}

inline uintptr_t find_module_base_dl(const char *soname)
{
    void *h = dlopen(soname, RTLD_NOLOAD);
    if (!h)
        h = dlopen(soname, RTLD_NOW);
    if (!h)
        return 0;
    Dl_info di{};
    void *sym = dlsym(h, "il2cpp_domain_get");
    if (!sym)
        sym = dlsym(h, "il2cpp_string_new");
    if (!sym)
        sym = dlsym(h, "UnityMain");
    if (!sym)
        sym = dlsym(h, "JNI_OnLoad");
    if (sym && dladdr(sym, &di) && di.dli_fbase)
        return reinterpret_cast<uintptr_t>(di.dli_fbase);
    return 0;
}

inline uintptr_t find_module_base(const char *needle)
{
    // Prefer linker truth (phdr/dlopen) over maps-lowest
    uintptr_t b = find_module_base_phdr(needle);
    if (b)
        return b;
    b = find_module_base_dl(needle);
    if (b)
        return b;
    return find_module_base_maps(needle);
}

inline int count_maps_matching(const char *needle)
{
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return -1;
    char line[768];
    int n = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (strstr(line, needle))
            ++n;
    }
    fclose(f);
    return n;
}

inline uintptr_t resolve_il2cpp_base()
{
    static const char *names[] = {"libil2cpp.so", "libil2cpp", "il2cpp.so"};
    for (const char *n : names)
    {
        uintptr_t b = find_module_base(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "il2cpp base=%p (%s)", (void *)b, n);
            return b;
        }
    }
    __android_log_print(ANDROID_LOG_ERROR, "xxx", "libil2cpp NOT FOUND maps=%d",
                        count_maps_matching("il2cpp"));
    return 0;
}

inline uintptr_t resolve_unity_base()
{
    static const char *names[] = {"libunity.so", "libunity"};
    for (const char *n : names)
    {
        uintptr_t b = find_module_base(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "unity base=%p (%s)", (void *)b, n);
            return b;
        }
    }
    __android_log_print(ANDROID_LOG_WARN, "xxx", "libunity NOT FOUND maps=%d",
                        count_maps_matching("unity"));
    return 0;
}

inline uintptr_t find_module_base_rx(const char *lib_name)
{
    return find_module_base(lib_name);
}
