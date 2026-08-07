#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <link.h>
#include <dlfcn.h>

namespace mods {

inline uintptr_t find_maps(const char* needle) {
    if (!needle || !needle[0]) return 0;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[768];
    uintptr_t best = 0;
    while (fgets(line, sizeof(line), f)) {
        uintptr_t start = 0, end = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &start, &end, perms) < 3)
            continue;
        // Melodium: any mapping with path match, take lowest start (ELF base)
        char* path = strchr(line, '/');
        if (!path) continue;
        if (!strstr(path, needle)) continue;
        if (!best || start < best) best = start;
    }
    fclose(f);
    return best;
}

inline uintptr_t find_phdr(const char* needle) {
    struct Ctx { const char* name; uintptr_t base; } ctx{needle, 0};
    dl_iterate_phdr(
        [](struct dl_phdr_info* info, size_t, void* data) -> int {
            auto* c = reinterpret_cast<Ctx*>(data);
            if (!info || !c->name) return 0;
            const char* n = info->dlpi_name ? info->dlpi_name : "";
            if (!n[0] || !strstr(n, c->name)) return 0;
            c->base = static_cast<uintptr_t>(info->dlpi_addr);
            return 1;
        },
        &ctx);
    return ctx.base;
}

inline uintptr_t find_dl(const char* soname) {
    void* h = dlopen(soname, RTLD_NOLOAD);
    if (!h) h = dlopen(soname, RTLD_NOW);
    if (!h) return 0;
    Dl_info di{};
    void* sym = dlsym(h, "il2cpp_domain_get");
    if (!sym) sym = dlsym(h, "il2cpp_string_new");
    if (!sym) sym = dlsym(h, "il2cpp_init");
    if (sym && dladdr(sym, &di) && di.dli_fbase)
        return reinterpret_cast<uintptr_t>(di.dli_fbase);
    return 0;
}

inline uintptr_t find_module(const char* needle) {
    if (uintptr_t b = find_phdr(needle)) return b;
    if (uintptr_t b = find_dl(needle)) return b;
    // also try plain soname for dl
    if (strstr(needle, "il2cpp")) {
        if (uintptr_t b = find_dl("libil2cpp.so")) return b;
    }
    return find_maps(needle);
}

inline uintptr_t resolve_il2cpp() {
    static const char* names[] = {"libil2cpp.so", "libil2cpp", "il2cpp.so"};
    for (const char* n : names) {
        if (uintptr_t b = find_module(n)) return b;
    }
    return 0;
}

} // namespace mods
