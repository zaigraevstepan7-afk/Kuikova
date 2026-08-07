#pragma once
// AArch64 inline hook — same approach Melodium/nova use for menu-critical hooks.

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

namespace a64hook {

constexpr size_t kPatchSize = 16;
constexpr uint32_t kLdrX16 = 0x58000050u;
constexpr uint32_t kBrX16  = 0xD61F0200u;

inline bool make_rwx(void* addr, size_t len) {
    const long ps = sysconf(_SC_PAGESIZE);
    const uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(ps - 1);
    const uintptr_t end = ((uintptr_t)addr + len + ps - 1) & ~(uintptr_t)(ps - 1);
    return mprotect((void*)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

inline bool make_rx(void* addr, size_t len) {
    const long ps = sysconf(_SC_PAGESIZE);
    const uintptr_t start = (uintptr_t)addr & ~(uintptr_t)(ps - 1);
    const uintptr_t end = ((uintptr_t)addr + len + ps - 1) & ~(uintptr_t)(ps - 1);
    return mprotect((void*)start, end - start, PROT_READ | PROT_EXEC) == 0;
}

inline bool already_patched(void* target) {
    if (!target) return false;
    auto* w = reinterpret_cast<uint32_t*>(target);
    return w[0] == kLdrX16 && w[1] == kBrX16;
}

inline bool install(void* target, void* replacement, void** out_trampoline) {
    if (!target || !replacement || !out_trampoline) return false;
    if (already_patched(target)) return false;

    constexpr size_t stub_size = 16 + 16;
    void* stub = mmap(nullptr, stub_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stub == MAP_FAILED) return false;

    auto* t = (uint8_t*)target;
    auto* s = (uint8_t*)stub;
    std::memcpy(s, t, 16);

    auto* back = (uint32_t*)(s + 16);
    back[0] = kLdrX16;
    back[1] = kBrX16;
    *reinterpret_cast<uint64_t*>(s + 16 + 8) = (uint64_t)(t + 16);
    __builtin___clear_cache((char*)s, (char*)s + stub_size);

    if (!make_rwx(target, kPatchSize)) {
        munmap(stub, stub_size);
        return false;
    }

    auto* patch = (uint32_t*)t;
    patch[0] = kLdrX16;
    patch[1] = kBrX16;
    *reinterpret_cast<uint64_t*>(t + 8) = (uint64_t)replacement;
    __builtin___clear_cache((char*)t, (char*)t + kPatchSize);
    make_rx(target, kPatchSize);

    *out_trampoline = stub;
    return true;
}

} // namespace a64hook
