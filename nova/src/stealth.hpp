#pragma once
#include "mem.hpp"

#include <cstdint>
#include <cstring>
#include <sys/prctl.h>

namespace stealth {

template <size_t N>
struct XorStr {
    char data[N]{};
    constexpr XorStr(const char (&s)[N]) {
        for (size_t i = 0; i < N; ++i) data[i] = static_cast<char>(s[i] ^ 0x5A);
    }
    void decode(char* out) const {
        for (size_t i = 0; i < N; ++i) out[i] = static_cast<char>(data[i] ^ 0x5A);
    }
};

#define XS(lit) ([]() -> const char* { \
    static constexpr ::stealth::XorStr<sizeof(lit)> _xs(lit); \
    static char _buf[sizeof(lit)]; \
    static bool _once = false; \
    if (!_once) { _xs.decode(_buf); _once = true; } \
    return _buf; \
}())

inline void disguise_thread(const char* name) {
    prctl(PR_SET_NAME, name ? name : "Binder:1_2", 0, 0, 0);
}

} // namespace stealth
