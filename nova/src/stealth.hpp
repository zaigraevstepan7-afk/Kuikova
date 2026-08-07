#pragma once
// Stealth helpers — reduce common AC signals. Not a guarantee against bans.

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <fstream>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/uio.h>
#include <unistd.h>

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

inline void* alloc_rw(size_t size) {
    void* p = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return p == MAP_FAILED ? nullptr : p;
}

inline bool seal_rx(void* p, size_t size) {
    return mprotect(p, size, PROT_READ | PROT_EXEC) == 0;
}

// Patch GOT entries in loaded .so rw segments that point at `symbol`.
inline int patch_got(void* symbol, void* replace) {
    if (!symbol || !replace) return 0;
    std::ifstream maps("/proc/self/maps");
    std::string line;
    int patched = 0;
    while (std::getline(maps, line)) {
        if (line.find("rw") == std::string::npos) continue;
        // Only patch likely callers of eglSwapBuffers (avoids slow/risky full scans)
        const bool interesting =
            line.find("libunity") != std::string::npos ||
            line.find("libmain") != std::string::npos ||
            line.find("libunity.so") != std::string::npos ||
            line.find("split_config") != std::string::npos ||
            line.find("libil2cpp") != std::string::npos;
        if (!interesting) continue;

        uintptr_t start = 0, end = 0;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;
        if (end <= start) continue;
        const size_t span = end - start;
        if (span > 8 * 1024 * 1024) continue;

        auto* p = reinterpret_cast<void**>(start);
        const size_t n = span / sizeof(void*);
        for (size_t i = 0; i < n; ++i) {
            void* cur = nullptr;
            iovec local{&cur, sizeof(cur)};
            iovec remote{&p[i], sizeof(cur)};
            if (process_vm_readv(getpid(), &local, 1, &remote, 1, 0) !=
                static_cast<ssize_t>(sizeof(cur)))
                continue;
            if (cur != symbol) continue;
            iovec wlocal{&replace, sizeof(replace)};
            iovec wremote{&p[i], sizeof(replace)};
            if (process_vm_writev(getpid(), &wlocal, 1, &wremote, 1, 0) ==
                static_cast<ssize_t>(sizeof(replace)))
                ++patched;
        }
    }
    return patched;
}

} // namespace stealth
