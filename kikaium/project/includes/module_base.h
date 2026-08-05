#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <cctype>
#include <link.h>
#include <dlfcn.h>
#include <android/log.h>

// True ELF load base = LOWEST mapped address of the library (not r-x text start).
// RVAs (TypeInfo / methods) are relative to this bias.
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
        // Path may contain spaces on some builds — take everything after offset/dev/inode
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &start, &end, perms) < 3)
            continue;
        char *path = strchr(line, '/');
        if (!path)
        {
            // apk!/ or memfd without leading slash — search whole line
            path = line;
        }
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
        sym = dlsym(h, "il2cpp_alloc");
    if (!sym)
        sym = dlsym(h, "UnityMain");
    // Any symbol from the SO — walk via dladdr
    if (!sym)
    {
        // force a known weak probe: dlsym of empty often fails; try common unity export
        sym = dlsym(h, "JNI_OnLoad");
    }
    if (sym && dladdr(sym, &di) && di.dli_fbase)
        return reinterpret_cast<uintptr_t>(di.dli_fbase);
    return 0;
}

inline uintptr_t find_module_base(const char *needle)
{
    uintptr_t b = find_module_base_maps(needle);
    if (b)
        return b;
    b = find_module_base_phdr(needle);
    if (b)
        return b;
    return 0;
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
    static const char *names[] = {
        "libil2cpp.so",
        "libil2cpp",
        "il2cpp.so",
        "il2cpp",
    };
    for (const char *n : names)
    {
        uintptr_t b = find_module_base(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "il2cpp base=%p via maps/phdr (%s)", (void *)b, n);
            return b;
        }
    }
    for (const char *n : names)
    {
        uintptr_t b = find_module_base_dl(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "il2cpp base=%p via dlopen (%s)", (void *)b, n);
            return b;
        }
    }
    __android_log_print(ANDROID_LOG_ERROR, "xxx",
                        "libil2cpp NOT FOUND maps_hits=%d", count_maps_matching("il2cpp"));
    return 0;
}

inline uintptr_t resolve_unity_base()
{
    static const char *names[] = {
        "libunity.so",
        "libunity",
        "unity.so",
    };
    for (const char *n : names)
    {
        uintptr_t b = find_module_base(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "unity base=%p (%s)", (void *)b, n);
            return b;
        }
    }
    for (const char *n : names)
    {
        uintptr_t b = find_module_base_dl(n);
        if (b)
        {
            __android_log_print(ANDROID_LOG_INFO, "xxx", "unity base=%p via dlopen (%s)", (void *)b, n);
            return b;
        }
    }
    __android_log_print(ANDROID_LOG_WARN, "xxx",
                        "libunity NOT FOUND maps_hits=%d", count_maps_matching("unity"));
    return 0;
}

// Back-compat alias
inline uintptr_t find_module_base_rx(const char *lib_name)
{
    return find_module_base(lib_name);
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
