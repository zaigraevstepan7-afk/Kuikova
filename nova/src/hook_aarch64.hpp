#pragma once
// GOT-only hook. No inline text patch (DIY trampolines crash eglSwapBuffers).

#include "stealth.hpp"

#include <cstdint>

namespace hook {

enum class Mode { Got, None };
inline Mode g_mode = Mode::None;

inline bool install_got(void* symbol, void* replace, void** original_out) {
    if (!symbol || !replace) return false;
    const int n = stealth::patch_got(symbol, replace);
    if (n <= 0) return false;
    // Original is the real function — call it by absolute address (bypasses GOT)
    if (original_out) *original_out = symbol;
    g_mode = Mode::Got;
    return true;
}

// Kept name for call sites; never does inline.
inline bool install(void* target, void* replace, void** original_out) {
    return install_got(target, replace, original_out);
}

inline bool install_inline(void* /*target*/, void* /*replace*/, void** /*original_out*/) {
    return false; // disabled — caused inject crashes
}

} // namespace hook
