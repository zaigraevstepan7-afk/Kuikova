#pragma once
// Halalium-style hook registry + getrr (OnStart) bypass — Dobby engine.
// OnStart @ RVA 0x8B9579C: DobbyDestroy tracked → call orig trampoline → DobbyHook reinstall.
// getrr itself is NOT tracked (same as egl / InputConsumer).

#include "dobby.h"

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
    void *orig{};
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
    uint32_t w = *reinterpret_cast<uint32_t *>(target);
    if (w == 0 || w == 0xffffffffu)
        return false;
    return true;
}

// Untracked DobbyHook (egl / InputConsumer / getrr)
inline bool install_untracked(void *target, void *replacement, void **orig_out)
{
    if (!target || !replacement)
        return false;
    return DobbyHook(target, replacement, orig_out) == 0;
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
        void *orig = nullptr;
        if (DobbyHook(target, replacement, &orig) != 0)
            return false;
        e.orig = orig;
        e.replacement = replacement;
        e.orig_out = orig_out;
        e.active = true;
        if (orig_out)
            *orig_out = orig;
        return true;
    }

    Entry e{};
    e.target = target;
    e.replacement = replacement;
    e.orig_out = orig_out;
    if (DobbyHook(target, replacement, &e.orig) != 0)
        return false;
    e.active = true;
    if (orig_out)
        *orig_out = e.orig;
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
        DobbyDestroy(e.target);
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
        void *orig = nullptr;
        if (DobbyHook(e.target, e.replacement, &orig) != 0)
        {
            __android_log_print(ANDROID_LOG_ERROR, "xxx_Bypass",
                                "reinstall DobbyHook failed @%p", e.target);
            continue;
        }
        e.orig = orig;
        e.active = true;
        if (e.orig_out)
            *e.orig_out = orig;
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
// Halalium: DobbyHook(OnStart, Bypass_getrr, &orig) — NOT tracked.
// Bypass: destroy tracked → blr orig trampoline → reinstall tracked.
constexpr uintptr_t kGetrrRva = 0x8B9579C;

inline void *&getrr_target()
{
    static void *t = nullptr;
    return t;
}

inline void *&getrr_orig()
{
    static void *o = nullptr;
    return o;
}

inline bool &getrr_armed()
{
    static bool v = false;
    return v;
}

inline bool &getrr_reentrant()
{
    thread_local bool v = false;
    return v;
}

using getrr_fn = void *(*)(void *self, int32_t p, void *method);

inline void *hk_getrr(void *self, int32_t p, void *method)
{
    if (getrr_reentrant())
    {
        auto *orig = (getrr_fn)getrr_orig();
        if (orig)
            return orig(self, p, method);
        return nullptr;
    }
    getrr_reentrant() = true;

    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass", "got call from getrr.");

    // 1) Hide tracked game hooks (Halalium DobbyDestroy loop)
    destroy_all();

    // 2) Call REAL OnStart via Dobby trampoline — site stays patched (Halalium blr orig)
    void *ret = nullptr;
    auto *orig = (getrr_fn)getrr_orig();
    if (orig)
        ret = orig(self, p, method);

    // 3) Reinstall tracked hooks (Halalium DobbyHook loop)
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

    void *orig = nullptr;
    if (DobbyHook(target, (void *)hk_getrr, &orig) != 0 || !orig)
    {
        __android_log_print(ANDROID_LOG_ERROR, "xxx_Bypass", "getrr DobbyHook failed @%p", target);
        return false;
    }

    getrr_target() = target;
    getrr_orig() = orig;
    getrr_armed() = true;
    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass",
                        "getrr OnStart DobbyHook OK rva=0x%lx @%p orig=%p",
                        (unsigned long)kGetrrRva, target, orig);
    return true;
}

inline bool uninstall_getrr_bypass()
{
    if (!getrr_armed() || !getrr_target())
        return true;
    DobbyDestroy(getrr_target());
    getrr_armed() = false;
    getrr_orig() = nullptr;
    __android_log_print(ANDROID_LOG_INFO, "xxx_Bypass", "getrr bypass OFF");
    return true;
}

inline bool getrr_is_armed()
{
    return getrr_armed();
}

} // namespace hhooks
