#pragma once
// Halalium-style hook registry + getrr (OnStart) bypass.
// When AC hits OnStart @ RVA 0x8B9579C Halalium:
//   1) logs "got call from getrr." (tag Halalium_Bypass)
//   2) DobbyDestroy every registered game hook
//   3) calls original OnStart
//   4) DobbyHook's them back
//   5) logs "bypas hok result %d"
// egl / input hooks are NOT in that list (same as Halalium).

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
    void **orig_out{}; // updated on (re)install
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

inline bool install_tracked(void *target, void *replacement, void **orig_out)
{
    if (!target || !replacement)
        return false;

    std::lock_guard<std::mutex> lock(mu());

    // Already tracked?
    for (auto &e : list())
    {
        if (e.target != target)
            continue;
        if (e.active)
            return true;
        // reinstall into existing slot
        if (!a64hook::install_with_backup(target, replacement, &e.trampoline, e.backup))
            return false;
        e.replacement = replacement;
        e.orig_out = orig_out;
        e.active = true;
        if (orig_out)
            *orig_out = e.trampoline;
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
        // Keep trampoline mapping — unused until reinstall replaces it.
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
        // Drop previous trampoline page if any
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

using getrr_fn = void *(*)(void *self, int32_t p, void *method);
inline getrr_fn &old_getrr()
{
    static getrr_fn f = nullptr;
    return f;
}

inline void *hk_getrr(void *self, int32_t p, void *method)
{
    __android_log_print(ANDROID_LOG_INFO, "Halalium_Bypass", "got call from getrr.");

    destroy_all();

    void *ret = nullptr;
    if (old_getrr())
        ret = old_getrr()(self, p, method);

    reinstall_all();

    __android_log_print(ANDROID_LOG_INFO, "Halalium_Bypass", "bypas hok result %d",
                        tracked_count());
    return ret;
}

// Install bypass itself — NOT added to the destroy list.
inline bool install_getrr_bypass(uintptr_t game_base)
{
    if (!game_base)
        return false;
    void *target = (void *)(game_base + kGetrrRva);
    void *tramp = nullptr;
    uint8_t unused_backup[a64hook::kPatchSize]{};
    if (!a64hook::install_with_backup(target, (void *)hk_getrr, &tramp, unused_backup))
    {
        __android_log_print(ANDROID_LOG_ERROR, "Halalium_Bypass", "getrr install failed @%p", target);
        return false;
    }
    old_getrr() = (getrr_fn)tramp;
    __android_log_print(ANDROID_LOG_INFO, "Halalium_Bypass", "getrr OnStart hook OK rva=0x%lx @%p",
                        (unsigned long)kGetrrRva, target);
    return true;
}

} // namespace hhooks
