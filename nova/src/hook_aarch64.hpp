#pragma once
// Minimal AArch64 inline hook (LDR X16 / BR X16 trampoline). Written from scratch.

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>

#define NOVA_LOG(...) __android_log_print(ANDROID_LOG_INFO, "nova", __VA_ARGS__)

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

struct Patch {
    void* target = nullptr;
    void* trampoline = nullptr;
    uint8_t saved[16]{};
};

inline bool install(void* target, void* replace, void** original_out, Patch* out_patch = nullptr) {
    if (!target || !replace) return false;

    // Trampoline: original 16 bytes + jump back to target+16
    void* tramp = mmap(nullptr, page_size(), PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (tramp == MAP_FAILED) return false;

    uint8_t stub[16];
    // LDR X16, #8 ; BR X16 ; .quad addr
    stub[0] = 0x50; stub[1] = 0x00; stub[2] = 0x00; stub[3] = 0x58; // LDR X16, #8
    stub[4] = 0x00; stub[5] = 0x02; stub[6] = 0x1F; stub[7] = 0xD6; // BR X16
    const uint64_t rep = reinterpret_cast<uint64_t>(replace);
    std::memcpy(stub + 8, &rep, 8);

    std::memcpy(tramp, target, 16);
    uint8_t back[16];
    back[0] = 0x50; back[1] = 0x00; back[2] = 0x00; back[3] = 0x58;
    back[4] = 0x00; back[5] = 0x02; back[6] = 0x1F; back[7] = 0xD6;
    const uint64_t ret = reinterpret_cast<uint64_t>(target) + 16;
    std::memcpy(back + 8, &ret, 8);
    std::memcpy(static_cast<uint8_t*>(tramp) + 16, back, 16);

    if (!protect(target, 16, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        munmap(tramp, page_size());
        return false;
    }

    uint8_t saved[16];
    std::memcpy(saved, target, 16);
    std::memcpy(target, stub, 16);
    __builtin___clear_cache(static_cast<char*>(target), static_cast<char*>(target) + 16);
    protect(target, 16, PROT_READ | PROT_EXEC);

    if (original_out) *original_out = tramp;
    if (out_patch) {
        out_patch->target = target;
        out_patch->trampoline = tramp;
        std::memcpy(out_patch->saved, saved, 16);
    }
    return true;
}

} // namespace hook
