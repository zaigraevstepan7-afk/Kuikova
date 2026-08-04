#pragma once
// Halalium-style hook registry + getrr (OnStart) bypass.
// OnStart @ RVA 0x8B9579C: destroy tracked hooks → call real OnStart → reinstall.
// Critical: do NOT call OnStart via a stolen-byte trampoline (PC-relative crash).
// Instead temporarily restore OnStart bytes and call the real address (Dobby-like safe path).

#include "a64_inline_hook.h"

#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>
#include <android/log.h>

namespace hhooks {

struct Entry
{
    void *target{};
    void *replacement{};
    void **orig_out{};
    void *trampoline{};
    uint8_t backup[a64hook::kPatchSize]{};
    bool active{false};
};

inline std::mutex &mu()
{
    static std::mutex m;
    return m;
}

inline std::vector<Entry> &list()
{
    static std::vector<Entry> v;
    return v;
}

inline bool looks_like_a64(void *target)
{
    if (!target || ((uintptr_t)target & 3))
        return false;
    // First word should be a plausible A64 insn (not all-zero / all-ff filler).
    uint32_t w = *reinterpret_cast<uint32_t *>(target);
    if (w == 0 || w == 0xffffffffu)
        return false;
    return true;
}

inline bool install_tracked(void *target, void *replacement, void **orig_out)
{
    if (!target || !replacement || !looks_like_a64(target))
        return false;

    std::lock_guard<std::mutex> lock(mu());

    for (auto &e : list())
    {
        if (e.target != target)
            continue;
        if (e.active)
            return true;
        void *tramp = nullptr;
        if (!a64hook::install_with_backup(target, replacement, &tramp, e.backup))
            return false;
        if (e.trampoline && e.trampoline != tramp)
            a64hook::release_trampoline(e.trampoline);
        e.trampoline = tramp;
        e.replacement = replacement;
        e.orig_out = orig_out;
        e.active = true;
        if (orig_out)
            *orig_out = tramp;
        return true;
    }

    Entry e{};
    e.target = target;
    e.replacement = replacement;
    e.orig_out = orig_out;
    if (!a64hook::install_with_backup(target, replacement, &e.trampoline, e.backup))
        return false;
    e.active = true;
    if (orig_out)
        *orig_out = e.trampoline;
    list().push_back(e);
    return true;
}

inline void destroy_all()
{
    std::lock_guard<std::mutex> lock(mu());
    for (auto &e : list())
    {
        if (!e.active || !e.target)
            continue;
        a64hook::restore(e.target, e.backup);
        e.active = false;
    }
}

inline void reinstall_all()
{
    std::lock_guard<std::mutex> lock(mu());
    for (auto &e : list())
    {
        if (e.active || !e.target || !e.replacement)
            continue;
        void *tramp = nullptr;
        if (!a64hook::install_with_backup(e.target, e.replacement, &tramp, e.backup))
            continue;
        if (e.trampoline && e.trampoline != tramp)
            a64hook::release_trampoline(e.trampoline);
        e.trampoline = tramp;
        e.active = true;
        if (e.orig_out)
            *e.orig_out = tramp;
    }
}

inline int tracked_count()
{
    std::lock_guard<std::mutex> lock(mu());
    int n = 0;
    for (auto &e : list())
        if (e.active)
            n++;
    return n;
}

// --- getrr / OnStart bypass (Halalium RVA 0x8B9579C) ---
constexpr uintptr_t kGetrrRva = 0x8B9579C;

inline void *&getrr_target()
{
    static void *t = nullptr;
    return t;
}

inline uint8_t *getrr_backup()
{
    static uint8_t b[a64hook::kPatchSize]{};
    return b;
}

inline bool &getrr_armed()
{
    static bool v = false;
    return v;
}

inline bool &getrr_reentrant()
{
    // TLS so nested calls from other threads don't trip the process-global flag
    thread_local bool v = false;
    return v;
}

using getrr_fn = void *(*)(void *self, int32_t p, void *method);

inline void *hk_getrr(void *self, int32_t p, void *method)
{
    // Re-entrancy: call real OnStart while temporarily restored (no trampoline).
    if (getrr_reentrant())
    {
        void *target = getrr_target();
        if (target)
            return ((getrr_fn)target)(self, p, method);
        return nullptr;
    }
    getrr_reentrant() = true;

    // Prefer soft log during AC scan window (Halalium still logs, but keep noise low)
    // __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass", "got call from getrr.");

    // 1) Hide tracked game hooks from AC scan
    destroy_all();

    // 2) Temporarily restore OnStart itself, call REAL function (no trampoline)
    void *ret = nullptr;
    void *target = getrr_target();
    if (target && getrr_armed())
    {
        a64hook::restore(target, getrr_backup());
        getrr_armed() = false;

        ret = ((getrr_fn)target)(self, p, method);

        // 3) Re-arm OnStart hook (patch only — no trampoline)
        if (a64hook::patch_jump(target, (void *)hk_getrr, getrr_backup()))
            getrr_armed() = true;
        else
            __android_log_print(ANDROID_LOG_ERROR, "xxx_Bypass", "getrr re-arm failed");
    }

    // 4) Put game hooks back
    reinstall_all();

    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass", "bypas hok result %d",
                        tracked_count());

    getrr_reentrant() = false;
    return ret;
}

inline bool install_getrr_bypass(uintptr_t game_base)
{
    if (!game_base)
        return false;
    if (getrr_armed())
        return true;

    void *target = (void *)(game_base + kGetrrRva);
    if (!looks_like_a64(target))
    {
        __android_log_print(ANDROID_LOG_ERROR, "xxx_Bypass",
                            "getrr target %p not a64 — skipped", target);
        return false;
    }

    if (!a64hook::patch_jump(target, (void *)hk_getrr, getrr_backup()))
    {
        __android_log_print(ANDROID_LOG_ERROR, "xxx_Bypass", "getrr install failed @%p", target);
        return false;
    }

    getrr_target() = target;
    getrr_armed() = true;
    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass",
                        "getrr OnStart hook OK rva=0x%lx @%p (safe restore-call)",
                        (unsigned long)kGetrrRva, target);
    return true;
}

inline bool uninstall_getrr_bypass()
{
    if (!getrr_armed() || !getrr_target())
        return true;
    a64hook::restore(getrr_target(), getrr_backup());
    getrr_armed() = false;
    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass", "getrr bypass OFF");
    return true;
}

inline bool getrr_is_armed()
{
    return getrr_armed();
}

} // namespace hhooks
