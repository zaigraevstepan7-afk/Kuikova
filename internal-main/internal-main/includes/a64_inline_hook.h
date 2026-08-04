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

constexpr size_t kPatchSize = 16;
constexpr uint32_t kLdrX16 = 0x58000050u;
constexpr uint32_t kBrX16 = 0xD61F0200u;

inline bool already_patched(void *target)
{
    auto *w = reinterpret_cast<uint32_t *>(target);
    return w[0] == kLdrX16 && w[1] == kBrX16;
}

inline bool install(void *target, void *replacement, void **out_trampoline)
{
    if (!target || !replacement || !out_trampoline)
        return false;

    // Second inject into same process often re-patches already-hooked egl → crash.
    if (already_patched(target))
    {
        MELODIUM_HOOK_LOG("a64hook refuse double-patch @%p", target);
        return false;
    }

    size_t stub_size = 16 + 16;
    void *stub = mmap(nullptr, stub_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stub == MAP_FAILED)
        return false;

    auto *t = (uint8_t *)target;
    auto *s = (uint8_t *)stub;

    memcpy(s, t, 16);

    uint32_t *back = (uint32_t *)(s + 16);
    back[0] = kLdrX16;
    back[1] = kBrX16;
    *(uint64_t *)(s + 16 + 8) = (uint64_t)(t + 16);

    __builtin___clear_cache((char *)s, (char *)s + stub_size);

    if (!make_rwx(target, kPatchSize))
    {
        munmap(stub, stub_size);
        return false;
    }

    uint32_t *patch = (uint32_t *)t;
    patch[0] = kLdrX16;
    patch[1] = kBrX16;
    *(uint64_t *)(t + 8) = (uint64_t)replacement;

    __builtin___clear_cache((char *)t, (char *)t + kPatchSize);
    make_rx(target, kPatchSize);

    *out_trampoline = stub;
    MELODIUM_HOOK_LOG("a64hook installed target=%p hook=%p tramp=%p", target, replacement, stub);
    return true;
}

} // namespace a64hook
