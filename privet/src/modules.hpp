#pragma once
// Module bases: TypeInfo + il2cpp_* RVAs live on libil2cpp.so (NOT libunity).
// Original privet bound everything to libunity — wrong module for these RVAs.

#include "mem.hpp"
#include <cstdint>
#include <cstring>
#include <vector>
#include <mutex>
#include <link.h>
#include <dlfcn.h>

namespace mods {

inline uintptr_t& il2cpp() {
    static uintptr_t b = 0;
    return b;
}
inline uintptr_t& unity() {
    static uintptr_t b = 0;
    return b;
}

// Backward-compat alias used across original code paths after fix.
inline uintptr_t& base() { return il2cpp(); }

struct lf_s { const char* name; uintptr_t addr; };
inline int lf_cb(struct dl_phdr_info* i, size_t, void* d) {
    auto* f = (lf_s*)d;
    if (i->dlpi_name && strstr(i->dlpi_name, f->name)) {
        f->addr = (uintptr_t)i->dlpi_addr;
        return 1;
    }
    return 0;
}
inline uintptr_t find_lib(const char* n) {
    lf_s f{n, 0};
    dl_iterate_phdr(lf_cb, &f);
    return f.addr;
}

inline uintptr_t pick_from_maps(const char* needle, uint64_t max_span = 0) {
    size_t n = 0;
    char* buf = pmem::load_maps_full(&n);
    if (!buf) return 0;
    uintptr_t best = 0;
    char* p = buf;
    while (*p) {
        char* le = p;
        while (*le && *le != '\n') le++;
        if (*le) *le = 0;
        uint64_t st = 0, en = 0;
        int k = 0;
        while (pmem::ishex(p[k])) { st = st * 16 + (uint64_t)pmem::hexv(p[k]); k++; }
        if (p[k] == '-') {
            k++;
            while (pmem::ishex(p[k])) { en = en * 16 + (uint64_t)pmem::hexv(p[k]); k++; }
        }
        while (p[k] == ' ') k++;
        if (p[k] == 'r' && p[k + 3] == 'p') {
            const char* q = p;
            while (*q && *q != '/') q++;
            if (*q == '/' && strstr(q, needle)) {
                if (!max_span || (en - st) < max_span) {
                    if (!best || st < best) best = (uintptr_t)st;
                }
            }
        }
        p = le + 1;
    }
    free(buf);
    return best;
}

inline void resolve_bases() {
    if (!il2cpp()) {
        il2cpp() = pick_from_maps("libil2cpp.so");
        if (!il2cpp()) il2cpp() = find_lib("libil2cpp.so");
        if (!il2cpp()) il2cpp() = find_lib("libil2cpp");
    }
    if (!unity()) {
        unity() = pick_from_maps("libunity.so", 0x5100000);
        if (!unity()) unity() = find_lib("libunity.so");
    }
}

// --- RVA resolve against libil2cpp segments (API + TypeInfo) ---
struct lib_seg { uint64_t va, foff, size, vaddr; bool exec, forge; };
inline std::vector<lib_seg>& segs() {
    static std::vector<lib_seg> s;
    return s;
}
inline std::mutex& segs_mu() {
    static std::mutex m;
    return m;
}

inline void build_segs_for(const char* soname) {
    std::vector<lib_seg> tmp;
    size_t n = 0;
    char* buf = pmem::load_maps_full(&n);
    if (!buf) return;
    char* p = buf;
    while (*p) {
        char* le = p;
        while (*le && *le != '\n') le++;
        if (*le) *le = 0;
        uint64_t st = 0, en = 0, off = 0;
        int k = 0;
        while (pmem::ishex(p[k])) { st = st * 16 + (uint64_t)pmem::hexv(p[k]); k++; }
        if (p[k] == '-') {
            k++;
            while (pmem::ishex(p[k])) { en = en * 16 + (uint64_t)pmem::hexv(p[k]); k++; }
        }
        while (p[k] == ' ') k++;
        bool exec = (p[k] == 'r' && p[k + 2] == 'x');
        for (int i = 0; i < 4 && p[k]; i++) k++;
        while (p[k] == ' ') k++;
        while (pmem::ishex(p[k])) { off = off * 16 + (uint64_t)pmem::hexv(p[k]); k++; }
        const char* q = p;
        while (*q && *q != '/') q++;
        if (*q == '/' && strstr(q, soname) && en > st)
            tmp.push_back({st, off, en - st, 0, exec, false});
        p = le + 1;
    }
    free(buf);

    uint64_t max_end = 0;
    for (auto& s : tmp)
        if (s.foff + s.size > max_end) max_end = s.foff + s.size;

    uint64_t load_bias = 0;
    for (auto& s : tmp) {
        if (s.foff == 0) {
            if (max_end && s.foff + s.size == max_end) s.forge = true;
            if (!s.forge) load_bias = s.va;
        }
    }
    if (!load_bias)
        for (auto& s : tmp)
            if (s.foff == 0 && !s.forge && (!load_bias || s.va < load_bias)) load_bias = s.va;
    if (!load_bias && !tmp.empty()) load_bias = tmp[0].va;

    for (auto& s : tmp)
        if (!s.forge) s.vaddr = s.va - load_bias;

    std::lock_guard<std::mutex> lk(segs_mu());
    segs().swap(tmp);
}

inline void build_segs() { build_segs_for("libil2cpp.so"); }

inline void* segment_resolve_rva(uint64_t rva) {
    std::lock_guard<std::mutex> lk(segs_mu());
    if (segs().empty()) return nullptr;
    void* any = nullptr;
    for (size_t i = 0; i < segs().size(); i++) {
        const lib_seg& s = segs()[i];
        if (s.forge || !s.size) continue;
        if (rva < s.vaddr || rva >= s.vaddr + s.size) continue;
        void* a = (void*)(s.va + (rva - s.vaddr));
        if (s.exec) return a;
        if (!any) any = a;
    }
    return any;
}

} // namespace mods
