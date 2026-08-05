#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <link.h>
#include <dlfcn.h>
#include <android/log.h>

// Resolve r-x mapping base for a shared library by substring match in maps.
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

// dl_iterate_phdr — works when maps path is apk!/memfd/split weirdness
inline uintptr_t find_module_base_phdr(const char *lib_name)
{
    struct Ctx
    {
        const char *name;
        uintptr_t base;
    } ctx{lib_name, 0};

    dl_iterate_phdr(
        [](struct dl_phdr_info *info, size_t, void *data) -> int {
            auto *c = reinterpret_cast<Ctx *>(data);
            if (!info || !info->dlpi_name || !c->name)
                return 0;
            if (!strstr(info->dlpi_name, c->name))
                return 0;
            c->base = static_cast<uintptr_t>(info->dlpi_addr);
            return 1;
        },
        &ctx);
    return ctx.base;
}

inline uintptr_t find_module_base(const char *lib_name)
{
    uintptr_t b = find_module_base_rx(lib_name);
    if (b)
        return b;
    b = find_module_base_phdr(lib_name);
    if (b)
        return b;
    // Last resort: dlopen + link_map
    void *h = dlopen(lib_name, RTLD_NOLOAD);
    if (!h)
        h = dlopen(lib_name, RTLD_NOW);
    if (h)
    {
        Dl_info di{};
        // pick any exported-ish symbol via dlsym of a common name, or just iterate
        void *sym = dlsym(h, "il2cpp_domain_get");
        if (!sym)
            sym = dlsym(h, "UnityMain");
        if (sym && dladdr(sym, &di) && di.dli_fbase)
            return reinterpret_cast<uintptr_t>(di.dli_fbase);
    }
    return 0;
}

inline uintptr_t resolve_il2cpp_base()
{
    uintptr_t b = find_module_base("libil2cpp.so");
    if (!b)
        b = find_module_base("libil2cpp");
    if (b)
    {
        __android_log_print(ANDROID_LOG_INFO, "xxx", "il2cpp base=%p", (void *)b);
        return b;
    }
    __android_log_print(ANDROID_LOG_ERROR, "xxx", "libil2cpp.so NOT FOUND");
    return 0;
}

inline uintptr_t resolve_unity_base()
{
    uintptr_t b = find_module_base("libunity.so");
    if (!b)
        b = find_module_base("libunity");
    if (b)
        __android_log_print(ANDROID_LOG_INFO, "xxx", "unity base=%p", (void *)b);
    else
        __android_log_print(ANDROID_LOG_WARN, "xxx", "libunity.so not found - RVA hooks disabled");
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
