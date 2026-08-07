#pragma once
// Hook eglSwapBuffers via ELF DT_JMPREL by symbol NAME.
// Matching by resolved pointer fails when GOT still holds the lazy PLT stub.

#include "mem.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <elf.h>
#include <cstdio>

namespace elfhook {

inline bool maps_exec_paths(std::vector<std::pair<uintptr_t, std::string>>& out) {
    out.clear();
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        uintptr_t start = 0, end = 0, off = 0;
        char perms[8]{}, path[512]{};
        if (sscanf(line.c_str(), "%lx-%lx %7s %lx %*s %*s %511s",
                   &start, &end, perms, &off, path) < 5)
            continue;
        if (std::strchr(perms, 'x') == nullptr) continue;
        if (path[0] != '/') continue;
        if (off != 0) continue; // ELF base mapping
        out.emplace_back(start, path);
    }
    return !out.empty();
}

inline bool interesting(const std::string& p) {
    return p.find("libunity") != std::string::npos ||
           p.find("libmain") != std::string::npos ||
           p.find("libEGL.so") != std::string::npos ||
           p.find("libGLESv") != std::string::npos ||
           p.find("split_config") != std::string::npos ||
           p.find("libil2cpp") != std::string::npos;
}

inline uintptr_t abs_or_base(uintptr_t base, uintptr_t v) {
    if (!v) return 0;
    return (v >= base) ? v : (base + v);
}

inline int hook_named_in_module(uintptr_t base, const char* sym_name,
                                void* replace, void** out_original_slot_value) {
    if (!base || !sym_name || !replace) return 0;

    Elf64_Ehdr ehdr{};
    if (!mem::read_into(base, ehdr)) return 0;
    if (std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) return 0;
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) return 0;
    if (ehdr.e_phnum == 0 || ehdr.e_phnum > 64) return 0;

    std::vector<Elf64_Phdr> phdrs(ehdr.e_phnum);
    if (!mem::read_bytes(base + ehdr.e_phoff, phdrs.data(),
                         phdrs.size() * sizeof(Elf64_Phdr)))
        return 0;

    uintptr_t dyn_vaddr = 0;
    size_t dyn_sz = 0;
    for (const auto& ph : phdrs) {
        if (ph.p_type == PT_DYNAMIC) {
            dyn_vaddr = ph.p_vaddr;
            dyn_sz = ph.p_memsz;
            break;
        }
    }
    if (!dyn_vaddr || dyn_sz < sizeof(Elf64_Dyn) || dyn_sz > 0x200000) return 0;

    const size_t ndyn = dyn_sz / sizeof(Elf64_Dyn);
    std::vector<Elf64_Dyn> dyns(ndyn);
    if (!mem::read_bytes(abs_or_base(base, dyn_vaddr), dyns.data(),
                         ndyn * sizeof(Elf64_Dyn)))
        return 0;

    uintptr_t strtab = 0, symtab = 0, jmprel = 0, rela = 0;
    size_t syment = sizeof(Elf64_Sym), relasz = 0, pltrelsz = 0, relaent = sizeof(Elf64_Rela);
    size_t strsz = 0;

    for (const auto& d : dyns) {
        if (d.d_tag == DT_NULL) break;
        switch (d.d_tag) {
        case DT_STRTAB: strtab = d.d_un.d_ptr; break;
        case DT_SYMTAB: symtab = d.d_un.d_ptr; break;
        case DT_STRSZ: strsz = d.d_un.d_val; break;
        case DT_SYMENT: syment = d.d_un.d_val; break;
        case DT_JMPREL: jmprel = d.d_un.d_ptr; break;
        case DT_PLTRELSZ: pltrelsz = d.d_un.d_val; break;
        case DT_RELA: rela = d.d_un.d_ptr; break;
        case DT_RELASZ: relasz = d.d_un.d_val; break;
        case DT_RELAENT: relaent = d.d_un.d_val; break;
        default: break;
        }
    }
    if (!strtab || !symtab || syment == 0) return 0;

    const uintptr_t str_abs = abs_or_base(base, strtab);
    const uintptr_t sym_abs = abs_or_base(base, symtab);

    auto name_at = [&](uint32_t off) -> std::string {
        if (strsz && off >= strsz) return {};
        char buf[128]{};
        if (!mem::read_bytes(str_abs + off, buf, sizeof(buf) - 1)) return {};
        buf[sizeof(buf) - 1] = 0;
        return std::string(buf);
    };

    auto patch_rela = [&](uintptr_t table_raw, size_t bytes) -> int {
        if (!table_raw || !bytes || !relaent) return 0;
        if (bytes > 0x400000) return 0;
        const uintptr_t table = abs_or_base(base, table_raw);
        const size_t n = bytes / relaent;
        int hits = 0;
        for (size_t i = 0; i < n; ++i) {
            Elf64_Rela r{};
            if (!mem::read_into(table + i * relaent, r)) continue;
            const uint32_t sym_idx = ELF64_R_SYM(r.r_info);
            Elf64_Sym sym{};
            if (!mem::read_into(sym_abs + static_cast<uintptr_t>(sym_idx) * syment, sym))
                continue;
            const std::string nm = name_at(sym.st_name);
            if (nm != sym_name) continue;

            uintptr_t where = r.r_offset;
            if (where < base) where = base + r.r_offset;

            const uintptr_t prev = mem::read_ptr(where);
            if (out_original_slot_value && prev &&
                prev != reinterpret_cast<uintptr_t>(replace)) {
                // Prefer a value that looks like a real code pointer; else caller uses dlsym
                *out_original_slot_value = reinterpret_cast<void*>(prev);
            }
            if (mem::write_ptr(where, reinterpret_cast<uintptr_t>(replace)))
                ++hits;
        }
        return hits;
    };

    int total = 0;
    total += patch_rela(jmprel, pltrelsz);
    total += patch_rela(rela, relasz);
    return total;
}

// Returns number of GOT/PLT slots patched.
inline int hook_symbol_name(const char* name, void* replace, void** slot_original_opt) {
    if (!name || !replace) return 0;
    std::vector<std::pair<uintptr_t, std::string>> mods;
    if (!maps_exec_paths(mods)) return 0;

    int total = 0;
    void* slot_orig = nullptr;
    for (const auto& m : mods) {
        if (!interesting(m.second)) continue;
        void* so = nullptr;
        total += hook_named_in_module(m.first, name, replace, &so);
        if (so && !slot_orig) slot_orig = so;
    }
    if (slot_original_opt && slot_orig) *slot_original_opt = slot_orig;
    return total;
}

} // namespace elfhook
