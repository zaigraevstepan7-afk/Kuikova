#pragma once
// Halalium-style in-process memory access.
// After inject Halalium uses direct LDR/STR in the game address space
// (ldr [base, TypeInfoRVA], ldr [player, #0x160], strb [player, #0xd8])
// with null checks — NOT process_vm_readv/writev.

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <unistd.h>
#include <sys/mman.h>
#include <vector>
#include <mutex>
#include <android/log.h>

namespace hmem {

struct MapRange
{
    uintptr_t start{};
    uintptr_t end{};
    int prot{};
};

inline std::mutex &maps_mu()
{
    static std::mutex m;
    return m;
}

inline std::vector<MapRange> &maps_cache()
{
    static std::vector<MapRange> c;
    return c;
}

inline void refresh_maps_unlocked()
{
    std::vector<MapRange> next;
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f)
        return;
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t s = 0, e = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &s, &e, perms) < 3)
            continue;
        int prot = 0;
        if (perms[0] == 'r')
            prot |= PROT_READ;
        if (perms[1] == 'w')
            prot |= PROT_WRITE;
        if (perms[2] == 'x')
            prot |= PROT_EXEC;
        if (!prot)
            continue;
        next.push_back({s, e, prot});
    }
    fclose(f);
    maps_cache().swap(next);
}

inline void refresh_maps()
{
    std::lock_guard<std::mutex> lock(maps_mu());
    refresh_maps_unlocked();
}

inline int prot_of(uintptr_t addr, size_t size = 1)
{
    if (!addr || size == 0)
        return 0;
    const uintptr_t last = addr + size - 1;
    std::lock_guard<std::mutex> lock(maps_mu());
    if (maps_cache().empty())
        refresh_maps_unlocked();
    for (const auto &r : maps_cache())
    {
        if (addr >= r.start && last < r.end)
            return r.prot;
    }
    return 0;
}

inline bool readable(uintptr_t addr, size_t size = sizeof(void *))
{
    return (prot_of(addr, size) & PROT_READ) != 0;
}

inline bool writable(uintptr_t addr, size_t size = sizeof(void *))
{
    return (prot_of(addr, size) & PROT_WRITE) != 0;
}

// Direct LDR — same as Halalium after inject.
template <typename T>
inline bool read(uintptr_t addr, T &out)
{
    if (!readable(addr, sizeof(T)))
        return false;
    std::memcpy(&out, reinterpret_cast<const void *>(addr), sizeof(T));
    return true;
}

template <typename T>
inline T read_or(uintptr_t addr, T fallback = T{})
{
    T v{};
    return read(addr, v) ? v : fallback;
}

inline uintptr_t read_ptr(uintptr_t addr)
{
    return read_or<uintptr_t>(addr, 0);
}

// Direct STR — same as Halalium strb/str.
template <typename T>
inline bool write(uintptr_t addr, const T &value)
{
    if (!addr)
        return false;
    if (!writable(addr, sizeof(T)))
    {
        // Halalium still writes when the page is RW; if RWX/RO, try mprotect once.
        uintptr_t page = addr & ~static_cast<uintptr_t>(sysconf(_SC_PAGESIZE) - 1);
        const int cur = prot_of(page, 1);
        if (!(cur & PROT_READ))
            return false;
        if (mprotect(reinterpret_cast<void *>(page), static_cast<size_t>(sysconf(_SC_PAGESIZE)),
                     PROT_READ | PROT_WRITE) != 0)
            return false;
        refresh_maps();
    }
    std::memcpy(reinterpret_cast<void *>(addr), &value, sizeof(T));
    return true;
}

inline bool write_ptr(uintptr_t addr, uintptr_t value)
{
    return write(addr, value);
}

inline bool write_bytes(uintptr_t addr, const void *src, size_t n)
{
    if (!addr || !src || !n)
        return false;
    if (!writable(addr, n))
    {
        uintptr_t page = addr & ~static_cast<uintptr_t>(sysconf(_SC_PAGESIZE) - 1);
        if (mprotect(reinterpret_cast<void *>(page), static_cast<size_t>(sysconf(_SC_PAGESIZE)),
                     PROT_READ | PROT_WRITE) != 0)
            return false;
        refresh_maps();
    }
    std::memcpy(reinterpret_cast<void *>(addr), src, n);
    return true;
}

// Il2CppClass* = *(base + TypeInfoRVA)  — Halalium pattern.
inline void *typeinfo(uintptr_t module_base, uintptr_t typeinfo_rva)
{
    if (!module_base || !typeinfo_rva)
        return nullptr;
    return reinterpret_cast<void *>(read_ptr(module_base + typeinfo_rva));
}

// Instance from TypeInfo->static_fields (Il2Cpp 2021+: static_fields @ +0xB8).
inline void *typeinfo_instance(void *klass, size_t static_fields_off = 0xB8)
{
    if (!klass)
        return nullptr;
    const uintptr_t k = reinterpret_cast<uintptr_t>(klass);
    const uintptr_t sf = read_ptr(k + static_fields_off);
    if (!sf)
        return nullptr;
    return reinterpret_cast<void *>(read_ptr(sf));
}

template <typename T>
inline T field(void *obj, uintptr_t offset, T fallback = T{})
{
    if (!obj)
        return fallback;
    return read_or<T>(reinterpret_cast<uintptr_t>(obj) + offset, fallback);
}

template <typename T>
inline bool set_field(void *obj, uintptr_t offset, const T &value)
{
    if (!obj)
        return false;
    return write(reinterpret_cast<uintptr_t>(obj) + offset, value);
}

inline void *field_ptr(void *obj, uintptr_t offset)
{
    return reinterpret_cast<void *>(field<uintptr_t>(obj, offset, 0));
}

} // namespace hmem
