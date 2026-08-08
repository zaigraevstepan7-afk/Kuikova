#pragma once
// AArch64 inline hook with ADRP/ADR relocation in the trampoline.
// Plain memcpy of the first 16 bytes breaks PC-relative insns — LateUpdate /
// Update on IL2CPP almost always start with STP + ADRP, so we rewrite those.

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

namespace a64hook {

constexpr size_t kPatchSize = 16;
constexpr uint32_t kLdrX16 = 0x58000050u; // LDR X16, #8
constexpr uint32_t kBrX16  = 0xD61F0200u; // BR X16

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

inline bool is_adrp(uint32_t w) {
    return (w & 0x9F000000u) == 0x90000000u;
}
inline bool is_adr(uint32_t w) {
    return (w & 0x9F000000u) == 0x10000000u;
}
inline bool is_ldr_literal(uint32_t w) {
    return (w & 0x3B000000u) == 0x18000000u;
}
inline bool is_b_imm(uint32_t w) {
    return (w & 0x7C000000u) == 0x14000000u; // B / BL
}
inline bool is_cbz(uint32_t w) {
    return (w & 0x7E000000u) == 0x34000000u || (w & 0x7E000000u) == 0x36000000u;
}
inline bool is_bcond(uint32_t w) {
    return (w & 0xFF000010u) == 0x54000000u;
}

// Absolute load into Xd: MOVZ/MOVK x4 (safe, no PC-rel).
inline size_t emit_load64(uint32_t* out, uint32_t rd, uint64_t imm) {
    auto movk = [](uint32_t rd, uint16_t imm16, uint32_t hw, bool keep) -> uint32_t {
        // MOVZ: 0xD2800000, MOV K: 0xF2800000
        uint32_t op = keep ? 0xF2800000u : 0xD2800000u;
        return op | ((hw & 3u) << 21) | ((uint32_t)imm16 << 5) | (rd & 31u);
    };
    out[0] = movk(rd, (uint16_t)(imm), 0, false);
    out[1] = movk(rd, (uint16_t)(imm >> 16), 1, true);
    out[2] = movk(rd, (uint16_t)(imm >> 32), 2, true);
    out[3] = movk(rd, (uint16_t)(imm >> 48), 3, true);
    return 4;
}

inline uint64_t adrp_page(uint32_t w, uint64_t pc) {
    uint32_t immlo = (w >> 29) & 3u;
    uint32_t immhi = (w >> 5) & 0x7FFFFu;
    int64_t imm = (int64_t)((immhi << 2) | immlo);
    if (imm & (1 << 20)) imm -= (1LL << 21);
    return (pc & ~0xFFFull) + (uint64_t)(imm << 12);
}

inline uint64_t adr_target(uint32_t w, uint64_t pc) {
    uint32_t immlo = (w >> 29) & 3u;
    uint32_t immhi = (w >> 5) & 0x7FFFFu;
    int64_t imm = (int64_t)((immhi << 2) | immlo);
    if (imm & (1 << 20)) imm -= (1LL << 21);
    return pc + (uint64_t)imm;
}

// Build a trampoline that preserves semantics of the first 4 instructions.
// Returns bytes written into stub, or 0 on failure.
inline size_t build_trampoline(uint8_t* stub, size_t stub_cap, void* target) {
    auto* src = reinterpret_cast<uint32_t*>(target);
    uint64_t src_pc = (uint64_t)target;
    uint32_t* out = reinterpret_cast<uint32_t*>(stub);
    size_t n = 0;

    for (int i = 0; i < 4; i++) {
        uint32_t w = src[i];
        uint64_t pc = src_pc + (uint64_t)i * 4u;

        if (is_b_imm(w) || is_cbz(w) || is_bcond(w) || is_ldr_literal(w)) {
            // Rare in IL2CPP prologues; refuse rather than mis-relocate.
            return 0;
        }

        if (is_adrp(w)) {
            uint32_t rd = w & 31u;
            uint64_t page = adrp_page(w, pc);
            if ((n + 4) * 4 > stub_cap) return 0;
            n += emit_load64(out + n, rd, page);
            continue;
        }
        if (is_adr(w)) {
            uint32_t rd = w & 31u;
            uint64_t abs = adr_target(w, pc);
            if ((n + 4) * 4 > stub_cap) return 0;
            n += emit_load64(out + n, rd, abs);
            continue;
        }

        // Verbatim (STP/MOV/etc.)
        if ((n + 1) * 4 > stub_cap) return 0;
        out[n++] = w;
    }

    // Jump back to target+16
    if ((n + 4) * 4 + 8 > stub_cap) return 0;
    // LDR X16, #8 ; BR X16 ; .quad back
    out[n++] = kLdrX16;
    out[n++] = kBrX16;
    // pad to 8-byte align for the absolute address that LDR reads at +8 from first LDR
    // Our LDR is at offset n_start; literal is at insn+8.
    // After writing LDR+BR we need the quad immediately after those 8 bytes.
    *reinterpret_cast<uint64_t*>(out + n) = src_pc + 16;
    n += 2;
    return n * 4;
}

inline bool install(void* target, void* replacement, void** out_trampoline) {
    if (!target || !replacement || !out_trampoline) return false;
    if (already_patched(target)) return false;

    constexpr size_t stub_cap = 128;
    void* stub = mmap(nullptr, stub_cap, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stub == MAP_FAILED) return false;

    size_t used = build_trampoline((uint8_t*)stub, stub_cap, target);
    if (!used) {
        munmap(stub, stub_cap);
        return false;
    }
    __builtin___clear_cache((char*)stub, (char*)stub + used);

    if (!make_rwx(target, kPatchSize)) {
        munmap(stub, stub_cap);
        return false;
    }

    auto* t = (uint8_t*)target;
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
