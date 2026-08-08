#pragma once
// AArch64 inline hook — Halalium/Dobby-equivalent for tracked game RVAs.
// 16-byte LDR x16 / BR x16 patch + executable trampoline (stolen bytes + return).

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>

namespace a64hook {

#ifndef MELODIUM_HOOK_LOG
#define MELODIUM_HOOK_LOG(...) \
    __android_log_print(ANDROID_LOG_INFO, "kikaium", __VA_ARGS__)
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
    if (!target)
        return false;
    auto *w = reinterpret_cast<uint32_t *>(target);
    return w[0] == kLdrX16 && w[1] == kBrX16;
}

inline void release_trampoline(void *tramp)
{
    if (tramp)
        munmap(tramp, 16 + 16);
}

inline bool restore(void *target, const uint8_t backup[kPatchSize])
{
    if (!target || !backup)
        return false;
    if (!make_rwx(target, kPatchSize))
        return false;
    memcpy(target, backup, kPatchSize);
    __builtin___clear_cache((char *)target, (char *)target + kPatchSize);
    make_rx(target, kPatchSize);
    return true;
}

// Patch jump only (no trampoline). Used by getrr OnStart.
inline bool patch_jump(void *target, void *replacement, uint8_t backup[kPatchSize])
{
    if (!target || !replacement || !backup)
        return false;
    if (already_patched(target))
        return false;
    memcpy(backup, target, kPatchSize);
    if (!make_rwx(target, kPatchSize))
        return false;
    auto *t = (uint8_t *)target;
    uint32_t *patch = (uint32_t *)t;
    patch[0] = kLdrX16;
    patch[1] = kBrX16;
    *(uint64_t *)(t + 8) = (uint64_t)replacement;
    __builtin___clear_cache((char *)t, (char *)t + kPatchSize);
    make_rx(target, kPatchSize);
    return true;
}

inline bool install_with_backup(void *target, void *replacement, void **out_trampoline, uint8_t backup[kPatchSize])
{
    if (!target || !replacement || !out_trampoline || !backup)
        return false;
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

    memcpy(backup, t, kPatchSize);
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
    MELODIUM_HOOK_LOG("a64hook+backup installed target=%p hook=%p tramp=%p", target, replacement, stub);
    return true;
}

inline bool install(void *target, void *replacement, void **out_trampoline)
{
    uint8_t unused[kPatchSize]{};
    return install_with_backup(target, replacement, out_trampoline, unused);
}

} // namespace a64hook
