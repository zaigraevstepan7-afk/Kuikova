#pragma once
// Parse ELF RELA/GOT and patch only real eglSwapBuffers relocations.
// Never brute-scan rw memory for pointer matches (that corrupted heaps → crash).

#include "mem.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

namespace elfhook {

struct MapMod {
    uintptr_t start = 0;
    uintptr_t offset = 0;
    std::string path;
};

inline bool load_maps(std::vector<MapMod>& out) {
    out.clear();
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        uintptr_t start = 0, end = 0, off = 0;
        char perms[8]{}, path[512]{};
        // 72000000-73000000 r-xp 00000000 fd:00 123 /path/libunity.so
        if (sscanf(line.c_str(), "%lx-%lx %7s %lx %*s %*s %511s",
                   &start, &end, perms, &off, path) < 4)
            continue;
        if (std::strchr(perms, 'x') == nullptr) continue;
        if (path[0] != '/') continue;
        MapMod m;
        m.start = start;
        m.offset = off;
        m.path = path;
        out.push_back(std::move(m));
    }
    return !out.empty();
}

inline uintptr_t module_base_for_path(const std::vector<MapMod>& maps, const std::string& path) {
    uintptr_t best = 0;
    for (const auto& m : maps) {
        if (m.path != path) continue;
        // ELF base is mapping with file offset 0
        if (m.offset == 0) {
            if (!best || m.start < best) best = m.start;
        }
    }
    if (best) return best;
    for (const auto& m : maps) {
        if (m.path.find(path) == std::string::npos && m.path != path) continue;
        if (m.offset == 0 && (!best || m.start < best)) best = m.start;
    }
    return best;
}

inline bool path_interesting(const std::string& p) {
    return p.find("libunity") != std::string::npos ||
           p.find("libmain") != std::string::npos ||
           p.find("libEGL") != std::string::npos ||
           p.find("split_config") != std::string::npos;
}

inline int patch_rela_in_module(uintptr_t base, void* symbol, void* replace) {
    if (!base || !symbol || !replace) return 0;

    Elf64_Ehdr ehdr{};
    if (!mem::read_into(base, ehdr)) return 0;
    if (std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) return 0;
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) return 0;

    // Read program headers to find dynamic segment
    if (ehdr.e_phnum == 0 || ehdr.e_phnum > 64) return 0;
    std::vector<Elf64_Phdr> phdrs(ehdr.e_phnum);
    if (!mem::read_bytes(base + ehdr.e_phoff, phdrs.data(),
                         phdrs.size() * sizeof(Elf64_Phdr)))
        return 0;

    uintptr_t dyn_vaddr = 0;
    size_t dyn_filesz = 0;
    for (const auto& ph : phdrs) {
        if (ph.p_type == PT_DYNAMIC) {
            dyn_vaddr = ph.p_vaddr;
            dyn_filesz = ph.p_memsz;
            break;
        }
    }
    if (!dyn_vaddr || !dyn_filesz || dyn_filesz > 0x100000) return 0;

    const size_t ndyn = dyn_filesz / sizeof(Elf64_Dyn);
    std::vector<Elf64_Dyn> dyns(ndyn);
    if (!mem::read_bytes(base + dyn_vaddr, dyns.data(), ndyn * sizeof(Elf64_Dyn)))
        return 0;

    uintptr_t rela = 0, relasz = 0, relaent = sizeof(Elf64_Rela);
    uintptr_t jmprel = 0, pltrelsz = 0;
    for (const auto& d : dyns) {
        if (d.d_tag == DT_NULL) break;
        switch (d.d_tag) {
        case DT_RELA: rela = d.d_un.d_ptr; break;
        case DT_RELASZ: relasz = d.d_un.d_val; break;
        case DT_RELAENT: relaent = d.d_un.d_val; break;
        case DT_JMPREL: jmprel = d.d_un.d_ptr; break;
        case DT_PLTRELSZ: pltrelsz = d.d_un.d_val; break;
        default: break;
        }
    }

    auto patch_table = [&](uintptr_t table_raw, size_t bytes) -> int {
        if (!table_raw || !bytes || relaent == 0) return 0;
        if (bytes > 0x200000) return 0;
        // DT_* pointers may be absolute (already relocated) or vaddr
        const uintptr_t table = (table_raw >= base) ? table_raw : (base + table_raw);
        const size_t n = bytes / relaent;
        int hit = 0;
        for (size_t i = 0; i < n; ++i) {
            Elf64_Rela r{};
            if (!mem::read_into(table + i * relaent, r)) continue;
            uintptr_t where = r.r_offset;
            if (where < base) where = base + r.r_offset;
            const uintptr_t cur = mem::read_ptr(where);
            if (cur != reinterpret_cast<uintptr_t>(symbol)) continue;
            if (mem::write_ptr(where, reinterpret_cast<uintptr_t>(replace)))
                ++hit;
        }
        return hit;
    };

    int patched = 0;
    patched += patch_table(jmprel, pltrelsz); // PLT relocs — main target
    patched += patch_table(rela, relasz);     // other RELA
    return patched;
}

inline int hook_symbol(void* symbol, void* replace) {
    if (!symbol || !replace) return 0;
    std::vector<MapMod> maps;
    if (!load_maps(maps)) return 0;

    // Unique paths
    std::vector<std::string> paths;
    for (const auto& m : maps) {
        if (!path_interesting(m.path)) continue;
        bool seen = false;
        for (const auto& p : paths) if (p == m.path) { seen = true; break; }
        if (!seen) paths.push_back(m.path);
    }

    int total = 0;
    for (const auto& path : paths) {
        // Find ELF base (offset 0 mapping)
        uintptr_t base = 0;
        for (const auto& m : maps) {
            if (m.path == path && m.offset == 0) {
                base = m.start;
                break;
            }
        }
        if (!base) continue;
        total += patch_rela_in_module(base, symbol, replace);
    }
    return total;
}

} // namespace elfhook
