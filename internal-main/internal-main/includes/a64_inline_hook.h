#pragma once
// Minimal AArch64 inline hook (Halalium/Dobby-equivalent for one symbol).
// Backs up the first 16 bytes, patches an absolute LDR/BR trampoline into the
// target, and builds an executable stub that runs the original bytes then
// jumps back to target+16.

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>

namespace a64hook {

#ifndef MELODIUM_HOOK_LOG
#define MELODIUM_HOOK_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, "melodium", __VA_ARGS__)
#endif

inline void *page_of(void *p)
{
    long ps = sysconf(_SC_PAGESIZE);
    return (void *)((uintptr_t)p & ~(uintptr_t)(ps - 1));
}

inline bool make_rwx(void *addr, size_t len)
{
    long ps = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(ps - 1);
    uintptr_t end = ((uintptr_t)addr + len + ps - 1) & ~(uintptr_t)(ps - 1);
    return mprotect((void *)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

inline bool make_rx(void *addr, size_t len)
{
    long ps = sysconf(_SC_PAGESIZE);
    uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(ps - 1);
    uintptr_t end = ((uintptr_t)addr + len + ps - 1) & ~(uintptr_t)(ps - 1);
    return mprotect((void *)start, end - start, PROT_READ | PROT_EXEC) == 0;
}

// Patch layout at target (16 bytes):
//   LDR X16, #8
//   BR  X16
//   .quad replacement
constexpr size_t kPatchSize = 16;

inline bool install(void *target, void *replacement, void **out_trampoline)
{
    if (!target || !replacement || !out_trampoline)
        return false;

    // Allocate RWX trampoline: [orig 16 bytes][LDR X16,#8][BR X16][.quad target+16]
    size_t stub_size = 16 + 16;
    void *stub = mmap(nullptr, stub_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stub == MAP_FAILED)
        return false;

    auto *t = (uint8_t *)target;
    auto *s = (uint8_t *)stub;

    // Copy original prologue
    memcpy(s, t, 16);

    // Jump back to target+16
    // LDR X16, #8 ; BR X16 ; .quad back
    uint32_t *back = (uint32_t *)(s + 16);
    back[0] = 0x58000050; // LDR X16, #8
    back[1] = 0xD61F0200; // BR X16
    *(uint64_t *)(s + 16 + 8) = (uint64_t)(t + 16);

    __builtin___clear_cache((char *)s, (char *)s + stub_size);

    if (!make_rwx(target, kPatchSize))
    {
        munmap(stub, stub_size);
        return false;
    }

    // Patch target → replacement
    uint32_t *patch = (uint32_t *)t;
    patch[0] = 0x58000050; // LDR X16, #8
    patch[1] = 0xD61F0200; // BR X16
    *(uint64_t *)(t + 8) = (uint64_t)replacement;

    __builtin___clear_cache((char *)t, (char *)t + kPatchSize);
    make_rx(target, kPatchSize);

    *out_trampoline = stub;
    MELODIUM_HOOK_LOG("a64hook installed target=%p hook=%p tramp=%p", target, replacement, stub);
    return true;
}

} // namespace a64hook
