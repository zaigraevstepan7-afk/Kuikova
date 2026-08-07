#pragma once
// Crash-safe-ish maps gate + direct reads (same logic as original privet main.cpp).

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <mutex>
#include <unistd.h>

namespace pmem {

struct map_range { uint64_t start, end; };

inline std::vector<map_range>& maps() {
    static std::vector<map_range> m;
    return m;
}
inline std::mutex& maps_mu() {
    static std::mutex m;
    return m;
}

inline long sc6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    register long x8 __asm__("x8") = n;
    register long x0 __asm__("x0") = a1;
    register long x1 __asm__("x1") = a2;
    register long x2 __asm__("x2") = a3;
    register long x3 __asm__("x3") = a4;
    register long x4 __asm__("x4") = a5;
    register long x5 __asm__("x5") = a6;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory", "cc");
    return x0;
}

inline int maps_open(const char* p) {
    return (int)sc6(56, -100, (long)p, 0, 0, 0, 0);
}
inline void maps_close(int fd) { sc6(57, fd, 0, 0, 0, 0, 0); }
inline long maps_read(int fd, void* b, size_t c) {
    return sc6(63, fd, (long)b, (long)c, 0, 0, 0);
}

inline size_t read_file_raw(const char* path, char* buf, size_t cap) {
    int fd = maps_open(path);
    if (fd < 0) return 0;
    size_t tot = 0;
    while (tot < cap - 1) {
        long n = maps_read(fd, buf + tot, cap - 1 - tot);
        if (n <= 0) break;
        tot += (size_t)n;
    }
    maps_close(fd);
    buf[tot] = 0;
    return tot;
}

inline char* load_maps_full(size_t* out_len) {
    const size_t cap = 2u << 20;
    char* buf = (char*)malloc(cap);
    if (!buf) { *out_len = 0; return nullptr; }
    size_t n = read_file_raw("/proc/self/maps", buf, cap);
    *out_len = n;
    if (!n) { free(buf); return nullptr; }
    return buf;
}

inline int hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
inline int ishex(char c) { return hexv(c) >= 0; }

inline void build_maps() {
    std::vector<map_range> tmp;
    size_t n = 0;
    char* buf = load_maps_full(&n);
    if (!buf) return;
    char* p = buf;
    while (*p) {
        char* le = p;
        while (*le && *le != '\n') le++;
        if (*le) *le = 0;
        uint64_t st = 0, en = 0;
        int k = 0;
        while (ishex(p[k])) { st = st * 16 + (uint64_t)hexv(p[k]); k++; }
        if (p[k] == '-') {
            k++;
            while (ishex(p[k])) { en = en * 16 + (uint64_t)hexv(p[k]); k++; }
        }
        while (p[k] == ' ') k++;
        if (p[k] == 'r') tmp.push_back({st, en});
        p = le + 1;
    }
    free(buf);
    std::lock_guard<std::mutex> lk(maps_mu());
    maps().swap(tmp);
}

inline bool readable(uint64_t a, size_t n) {
    if (!a || n == 0 || a < 0x1000) return false;
    uint64_t end = a + n;
    if (end < a) return false;
    std::lock_guard<std::mutex> lk(maps_mu());
    for (size_t i = 0; i < maps().size(); i++) {
        if (a >= maps()[i].start && end <= maps()[i].end) return true;
        if (a < maps()[i].start) return false;
    }
    return false;
}

inline bool ok(uint64_t p) {
    return p > 0x10000 && p < 0x0000FFFFFFFFFFFFull;
}

inline uint64_t rd64(uint64_t a) { return readable(a, 8) ? *(uint64_t*)a : 0; }
inline int32_t  rd32(uint64_t a) { return readable(a, 4) ? *(int32_t*)a : 0; }
inline uint8_t  rd8(uint64_t a)  { return readable(a, 1) ? *(uint8_t*)a : 0; }
inline uint16_t rd16(uint64_t a) { return readable(a, 2) ? *(uint16_t*)a : 0; }
inline float    rdf(uint64_t a)  { return readable(a, 4) ? *(float*)a : 0.f; }
inline void wr8(uint64_t a, uint8_t v)  { if (readable(a, 1)) *(uint8_t*)a = v; }
inline void wr32(uint64_t a, int32_t v) { if (readable(a, 4)) *(int32_t*)a = v; }

inline bool rdb(uint64_t a, void* buf, size_t n) {
    if (!readable(a, n)) return false;
    memcpy(buf, (void*)a, n);
    return true;
}

inline bool obj_ok(uint64_t a) {
    if (!readable(a, 8)) return false;
    uint64_t k = *(uint64_t*)a;
    return readable(k, 8) && k > 0x1000000;
}

} // namespace pmem
