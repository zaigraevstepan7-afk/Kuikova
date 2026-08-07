#pragma once
#include "stealth.hpp"

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

namespace hook {

inline size_t page_size() {
    static size_t s = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    return s ? s : 4096;
}

inline bool protect(void* addr, size_t len, int prot) {
    const auto ps = page_size();
    const auto start = reinterpret_cast<uintptr_t>(addr) & ~(ps - 1);
    const auto end = (reinterpret_cast<uintptr_t>(addr) + len + ps - 1) & ~(ps - 1);
    return mprotect(reinterpret_cast<void*>(start), end - start, prot) == 0;
}

inline void emit_abs_jump(uint8_t* dst, uint64_t to) {
    dst[0] = 0x50; dst[1] = 0x00; dst[2] = 0x00; dst[3] = 0x58;
    dst[4] = 0x00; dst[5] = 0x02; dst[6] = 0x1F; dst[7] = 0xD6;
    std::memcpy(dst + 8, &to, 8);
}

inline bool install_inline(void* target, void* replace, void** original_out) {
    if (!target || !replace) return false;
    if (reinterpret_cast<uintptr_t>(target) & 3u) return false;

    const size_t ps = page_size();
    void* tramp = stealth::alloc_rw(ps);
    if (!tramp) return false;

    std::memcpy(tramp, target, 16);
    emit_abs_jump(static_cast<uint8_t*>(tramp) + 16,
                  reinterpret_cast<uint64_t>(target) + 16);
    if (!stealth::seal_rx(tramp, ps)) {
        munmap(tramp, ps);
        return false;
    }

    if (!protect(target, 16, PROT_READ | PROT_WRITE)) {
        munmap(tramp, ps);
        return false;
    }

    uint8_t stub[16];
    emit_abs_jump(stub, reinterpret_cast<uint64_t>(replace));
    std::memcpy(target, stub, 16);
    __builtin___clear_cache(static_cast<char*>(target), static_cast<char*>(target) + 16);
    protect(target, 16, PROT_READ | PROT_EXEC);

    if (original_out) *original_out = tramp;
    return true;
}

enum class Mode { Got, Inline, None };
inline Mode g_mode = Mode::None;

inline bool install(void* target, void* replace, void** original_out) {
    if (!target || !replace) return false;

    // 1) GOT first — avoids patching libEGL .text (common integrity check)
    const int n = stealth::patch_got(target, replace);
    if (n > 0) {
        if (original_out) *original_out = target; // call real symbol directly
        g_mode = Mode::Got;
        return true;
    }

    // 2) Inline fallback
    if (install_inline(target, replace, original_out)) {
        g_mode = Mode::Inline;
        return true;
    }
    g_mode = Mode::None;
    return false;
}

} // namespace hook
