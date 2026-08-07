//by @gametocytes
//by @gametocytes
//by @gametocytes
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <mutex>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <jni.h>
#include <atomic>

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"
#include "font.h"
#include "offsets.hpp"
#include "vector3.h"
#include "il2cpp.hpp"
#include "a64_inline_hook.hpp"

extern "C" {
__attribute__((visibility("hidden"), used)) void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    unsigned char v = (unsigned char)c;
    while (n--) *p++ = v;
    return s;
}
__attribute__((visibility("hidden"), used)) void* memcpy(void* d, const void* s, size_t n) {
    unsigned char* dd = (unsigned char*)d;
    const unsigned char* ss = (const unsigned char*)s;
    while (n--) *dd++ = *ss++;
    return d;
}
__attribute__((visibility("hidden"), used)) void* memmove(void* d, const void* s, size_t n) {
    unsigned char* dd = (unsigned char*)d;
    const unsigned char* ss = (const unsigned char*)s;
    if (dd < ss) { while (n--) *dd++ = *ss++; }
    else { dd += n; ss += n; while (n--) *--dd = *--ss; }
    return d;
}
__attribute__((visibility("hidden"), used)) int memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* x = (const unsigned char*)a;
    const unsigned char* y = (const unsigned char*)b;
    while (n--) { if (*x != *y) return (int)*x - (int)*y; x++; y++; }
    return 0;
}
__attribute__((visibility("hidden"), used)) void* memchr(const void* s, int c, size_t n) {
    const unsigned char* p = (const unsigned char*)s;
    unsigned char v = (unsigned char)c;
    while (n--) { if (*p == v) return (void*)p; p++; }
    return nullptr;
}
__attribute__((visibility("hidden"), used)) size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}
__attribute__((visibility("hidden"), used)) int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}
__attribute__((visibility("hidden"), used)) int strncmp(const char* a, const char* b, size_t n) {
    while (n && *a && *a == *b) { a++; b++; n--; }
    return n ? (int)(unsigned char)*a - (int)(unsigned char)*b : 0;
}
__attribute__((visibility("hidden"), used)) char* strstr(const char* h, const char* n) {
    if (!*n) return (char*)h;
    for (; *h; h++) {
        const char* a = h;
        const char* b = n;
        while (*b && *a == *b) { a++; b++; }
        if (!*b) return (char*)h;
    }
    return nullptr;
}
}

static uintptr_t g_base = 0;
static uintptr_t g_il2 = 0;
static bool opt_box = true, opt_health = true, opt_dist = true, opt_skeleton = false, opt_tps = false;
static bool opt_aa = false, opt_aa_jitter = false, opt_aa_chaos = false;
static int opt_aa_pitch = 0; // 0 local, 1 up, 2 down
static int opt_aa_yaw = 0;   // 0 local, 1 backward, 2 spiral, 3 chaos
static int opt_aa_range = 15;
static int opt_aa_frames = 2;
static float opt_aa_spin = 0.f;
static bool menu_open = false;
static float menu_alpha = 0.f;
static float tps_dist = 2.5f;
static std::atomic<bool> g_hooks_armed{false}; // PC Update/LateUpdate only after match/setup
static int scr_w = 0, scr_h = 0;
static std::mutex g_mtx;

static inline bool ok(uint64_t p) {
    return p > 0x10000 && p < 0x0000FFFFFFFFFFFFull;
}

static uintptr_t find_lib(const char* n);

struct map_range { uint64_t start, end; };
static std::vector<map_range> g_maps;
static std::mutex g_mtx_maps;

#include <sys/syscall.h>
static inline long sc6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
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
#define MAPS_AT_FDCWD (-100)
#define MAPS_SYS_OPENAT 56
#define MAPS_SYS_READ 63
#define MAPS_SYS_CLOSE 57
#define MAPS_O_RDONLY 0

static inline int maps_open(const char* p) { return (int)sc6(MAPS_SYS_OPENAT, MAPS_AT_FDCWD, (long)p, MAPS_O_RDONLY, 0, 0, 0); }
static inline void maps_close(int fd) { sc6(MAPS_SYS_CLOSE, fd, 0, 0, 0, 0, 0); }
static inline long maps_read(int fd, void* b, size_t c) { return sc6(MAPS_SYS_READ, fd, (long)b, c, 0, 0, 0); }

static size_t read_file_raw(const char* path, char* buf, size_t cap) {
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

static inline int hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static inline int ishex(char c) { return hexv(c) >= 0; }

static char* load_maps_full(size_t* out_len) {
    const size_t cap = 2u << 20;
    char* buf = (char*)malloc(cap);
    if (!buf) { *out_len = 0; return nullptr; }
    size_t n = read_file_raw("/proc/self/maps", buf, cap);
    *out_len = n;
    if (!n) { free(buf); return nullptr; }
    return buf;
}

static void build_maps() {
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
    std::lock_guard<std::mutex> lk(g_mtx_maps);
    g_maps.swap(tmp);
}

static inline bool readable(uint64_t a, size_t n) {
    if (!a || n == 0 || a < 0x1000) return false;
    uint64_t end = a + n;
    if (end < a) return false;
    std::lock_guard<std::mutex> lk(g_mtx_maps);
    for (size_t i = 0; i < g_maps.size(); i++) {
        if (a >= g_maps[i].start && end <= g_maps[i].end) return true;
        if (a < g_maps[i].start) return false;
    }
    return false;
}

static inline uint64_t rd64(uint64_t a) { return readable(a, 8) ? *(uint64_t*)a : 0; }
static inline int32_t rd32(uint64_t a) { return readable(a, 4) ? *(int32_t*)a : 0; }
static inline uint8_t rd8(uint64_t a) { return readable(a, 1) ? *(uint8_t*)a : 0; }
static inline uint16_t rd16(uint64_t a) { return readable(a, 2) ? *(uint16_t*)a : 0; }
static inline void wr8(uint64_t a, uint8_t v) { if (readable(a, 1)) *(uint8_t*)a = v; }
static inline void wr32(uint64_t a, int32_t v) { if (readable(a, 4)) *(int32_t*)a = v; }
static inline bool obj_ok(uint64_t a) {
    if (!readable(a, 8)) return false;
    uint64_t k = *(uint64_t*)a;
    return readable(k, 8) && k > 0x1000000;
}
static inline float rdf(uint64_t a) { return readable(a, 4) ? *(float*)a : 0.f; }
static inline void wrf(uint64_t a, float v) { if (readable(a, 4)) *(float*)a = v; }
static inline void rdv(uint64_t a, Vector3& v) {
    if (readable(a, sizeof(Vector3))) memcpy(&v, (void*)a, sizeof(Vector3));
    else memset(&v, 0, sizeof(Vector3));
}
static inline bool rdb(uint64_t a, void* buf, size_t n) {
    if (!readable(a, n)) return false;
    memcpy(buf, (void*)a, n);
    return true;
}
static inline bool sane_world_pos(const Vector3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) &&
           fabsf(v.x) < 100000.f && fabsf(v.y) < 100000.f && fabsf(v.z) < 100000.f &&
           !(v.x == 0.f && v.y == 0.f && v.z == 0.f);
}

struct lib_seg { uint64_t va, foff, size, vaddr; bool exec, forge; };
static std::vector<lib_seg> g_segs;
static std::mutex g_mtx_segs;

static void build_segs() {
    std::vector<lib_seg> tmp;
    size_t n = 0;
    char* buf = load_maps_full(&n);
    if (!buf) return;
    char* p = buf;
    while (*p) {
        char* le = p;
        while (*le && *le != '\n') le++;
        if (*le) *le = 0;
        uint64_t st = 0, en = 0, off = 0;
        int k = 0;
        while (ishex(p[k])) { st = st * 16 + (uint64_t)hexv(p[k]); k++; }
        if (p[k] == '-') {
            k++;
            while (ishex(p[k])) { en = en * 16 + (uint64_t)hexv(p[k]); k++; }
        }
        while (p[k] == ' ') k++;
        bool exec = (p[k] == 'r' && p[k + 2] == 'x');
        for (int i = 0; i < 4 && p[k]; i++) k++;
        while (p[k] == ' ') k++;
        while (ishex(p[k])) { off = off * 16 + (uint64_t)hexv(p[k]); k++; }
        const char* q = p;
        while (*q && *q != '/') q++;
        if (*q == '/' && strstr(q, "libil2cpp.so") && en > st)
            tmp.push_back({st, off, en - st, 0, exec, false});
        p = le + 1;
    }
    free(buf);

    uint64_t max_end = 0;
    for (auto& s : tmp)
        if (s.foff + s.size > max_end) max_end = s.foff + s.size;

    uint64_t load_bias = 0;
    for (auto& s : tmp) {
        if (s.foff == 0) {
            if (max_end && s.foff + s.size == max_end) s.forge = true;
            if (!s.forge) load_bias = s.va;
        }
    }
    if (!load_bias)
        for (auto& s : tmp)
            if (s.foff == 0 && !s.forge && (!load_bias || s.va < load_bias)) load_bias = s.va;
    if (!load_bias && !tmp.empty()) load_bias = tmp[0].va;

    for (auto& s : tmp)
        if (!s.forge) s.vaddr = s.va - load_bias;

    std::lock_guard<std::mutex> lk(g_mtx_segs);
    g_segs.swap(tmp);
}

static void* segment_resolve_rva(uint64_t rva) {
    std::lock_guard<std::mutex> lk(g_mtx_segs);
    if (g_segs.empty()) {
        // unlock not possible with lock_guard; rebuild outside would race — call once at init
        return nullptr;
    }
    void* any = nullptr;
    for (size_t i = 0; i < g_segs.size(); i++) {
        const lib_seg& s = g_segs[i];
        if (s.forge || !s.size) continue;
        if (rva < s.vaddr || rva >= s.vaddr + s.size) continue;
        void* a = (void*)(s.va + (rva - s.vaddr));
        if (s.exec) return a;
        if (!any) any = a;
    }
    return any;
}

// Melodium UnityEngine.Touch (return-by-value ABI; compiler emits sret on arm64).
struct UnityTouch {
    int32_t fingerId;
    float px, py;
    float rpx, rpy;
    float dpx, dpy;
    float deltaTime;
    int32_t tapCount;
    int32_t phase;
    int32_t type;
    float pressure;
    float maxPressure;
    float radius;
    float radiusVar;
    float altAngle;
    float azAngle;
};

static int (*touch_count_fn)() = nullptr;
static UnityTouch (*get_touch_fn)(int) = nullptr;

// InputConsumer feed (Halalium/Melodium overlay path) — applied Melodium-style after NewFrame.
static std::mutex g_touch_mu;
static float g_touch_x = 0.f, g_touch_y = 0.f;
static bool g_touch_down = false;
static bool g_touch_have = false;

static void feed_touch(float x, float y, bool down) {
    std::lock_guard<std::mutex> lk(g_touch_mu);
    g_touch_x = x;
    g_touch_y = y;
    g_touch_down = down;
    g_touch_have = true;
}

static uintptr_t resolve_il2() {
    uintptr_t b = find_lib("libil2cpp.so");
    if (!b) b = find_lib("libil2cpp");
    if (!b) {
        // maps fallback
        size_t n = 0;
        char* buf = load_maps_full(&n);
        if (buf) {
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
                if (p[k] == 'r' && p[k + 3] == 'x') {
                    const char* q = p;
                    while (*q && *q != '/') q++;
                    if (*q == '/' && strstr(q, "libil2cpp.so") && en > st) {
                        free(buf);
                        return st;
                    }
                }
                p = le + 1;
            }
            free(buf);
        }
    }
    return b;
}

static bool touch_init() {
    if (!g_il2) g_il2 = resolve_il2();
    if (!g_il2) return false;
    touch_count_fn = (int (*)())(g_il2 + OFF_INPUT_GET_TOUCH_COUNT);
    get_touch_fn = (UnityTouch (*)(int))(g_il2 + OFF_INPUT_GET_TOUCH);
    return touch_count_fn && get_touch_fn;
}

static const Il2CppMethod* find_method(Il2CppClass* c, const char* name) {
    if (!c || !il2cpp::class_get_methods || !il2cpp::method_get_name) return nullptr;
    for (Il2CppClass* k = c; k && il2cpp::class_get_parent; k = il2cpp::class_get_parent(k)) {
        void* it = nullptr;
        while (const Il2CppMethod* m = il2cpp::class_get_methods(k, &it)) {
            const char* nm = il2cpp::method_get_name(m);
            if (nm && strcmp(nm, name) == 0) return m;
        }
        if (k == c && il2cpp::class_get_method_from_name) {
            const Il2CppMethod* m = il2cpp::class_get_method_from_name(c, name, 0);
            if (m) return m;
        }
    }
    return nullptr;
}

// Melodium order: call AFTER ImGui::NewFrame(), write MousePos/MouseDown directly.
static void handle_touch() {
    auto& io = ImGui::GetIO();
    static bool touch_active = false;

    bool have = false;
    float x = 0.f, y = 0.f;
    bool down = false;
    {
        std::lock_guard<std::mutex> lk(g_touch_mu);
        if (g_touch_have) {
            x = g_touch_x;
            y = g_touch_y;
            down = g_touch_down;
            have = true;
        }
    }

    if (!have) {
        if (!touch_count_fn || !get_touch_fn) return;
        int touch_count = touch_count_fn();
        if (touch_count <= 0) {
            if (touch_active) {
                io.MouseDown[0] = false;
                touch_active = false;
            }
            return;
        }
        for (int i = 0; i < touch_count; i++) {
            UnityTouch it = get_touch_fn(i);
            int phase = it.phase;
            x = it.px;
            y = io.DisplaySize.y - it.py;
            if (phase == 0 || phase == 1 || phase == 2) {
                io.MousePos = ImVec2(x, y);
                io.MouseDown[0] = true;
                touch_active = true;
                return;
            }
            if (phase == 3 || phase == 4) {
                io.MouseDown[0] = false;
                touch_active = false;
            }
        }
        return;
    }

    // InputConsumer path (screen space, top-left origin — no Y flip)
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = down;
    touch_active = down;
}

static bool str_contains(uint64_t s, const char* needle) {
    if (!ok(s)) return false;
    int len = rd32(s + OFF_UNITY_STRING_LENGTH);
    if (len <= 0 || len > 200) return false;
    char buf[220];
    int n = len < 219 ? len : 219;
    for (int i = 0; i < n; i++) {
        uint16_t c = rd16(s + OFF_UNITY_STRING_CHARS + 2 * (uint64_t)i);
        buf[i] = (c < 0x80) ? (char)c : '?';
    }
    buf[n] = 0;
    return strstr(buf, needle) != nullptr;
}

static int health_of(uint64_t p) {
    float hp = rdf(p + OFF_PLAYER_HEALTH);
    if (hp > 0.f && hp < 1000.f) return (int)hp;
    uint64_t pp = rd64(p + OFF_PLAYER_PHOTON_PTR);
    if (!ok(pp)) return 0;
    uint64_t pr = rd64(pp + OFF_PHOTON_PROPS_REG);
    if (!ok(pr)) return 0;
    int cnt = rd32(pr + OFF_PROPS_COUNT);
    if (cnt <= 0 || cnt > 4096) return 0;
    uint64_t pl = rd64(pr + OFF_PROPS_LIST);
    if (!ok(pl)) return 0;
    for (int i = 0; i < cnt; i++) {
        uint64_t k = rd64(pl + OFF_PROPS_KEY_BASE + 0x18 * (uint64_t)i);
        if (!ok(k)) continue;
        if (str_contains(k, "health")) {
            uint64_t v = rd64(pl + OFF_PROPS_VAL_BASE + 0x18 * (uint64_t)i);
            if (ok(v)) {
                int val = rd32(v + OFF_PROPS_VALUE_DATA);
                if (val > 0 && val < 1000) return val;
            }
        }
    }
    return 0;
}

static uint64_t pm_lazy();

struct lf_s { const char* name; uintptr_t addr; };
static int lf_cb(struct dl_phdr_info* i, size_t s, void* d) {
    (void)s;
    lf_s* f = (lf_s*)d;
    if (i->dlpi_name && strstr(i->dlpi_name, f->name)) { f->addr = i->dlpi_addr; return 1; }
    return 0;
}
static uintptr_t find_lib(const char* n) {
    lf_s f{n, 0};
    dl_iterate_phdr(lf_cb, &f);
    return f.addr;
}

static uintptr_t pick_base() {
    size_t n = 0;
    char* buf = load_maps_full(&n);
    if (buf) {
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
            if (p[k] == 'r' && p[k + 3] == 'p') {
                const char* q = p;
                while (*q && *q != '/') q++;
                if (*q == '/' && strstr(q, "libunity.so") && (en - st) < 0x5100000) {
                    free(buf);
                    return st;
                }
            }
            p = le + 1;
        }
        free(buf);
    }
    return find_lib("libunity.so");
}

static bool valid_pm(uint64_t pm) {
    return ok(pm) && ok(rd64(pm + OFF_PM_LOCAL_PLAYER));
}

static uint64_t pm_lazy_at(uint64_t base) {
    uint64_t v1 = rd64(base + OFF_PLAYER_MANAGER);
    if (!ok(v1)) return 0;
    uint64_t v2 = rd64(v1 + 0x58);
    if (!ok(v2)) return 0;
    uint64_t v3 = rd64(v2 + 0xB8);
    if (!ok(v3)) return 0;
    return rd64(v3 + 0x0);
}

static uint64_t pm_static_at(uint64_t base) {
    uint64_t cls = rd64(base + OFF_PLAYER_MANAGER);
    if (!ok(cls)) return 0;
    uint64_t obj = rd64(cls + 0x90);
    if (!ok(obj)) return 0;
    return rd64(obj + 0x10);
}

static uint64_t pm_lazy() { return pm_lazy_at(g_base); }
static uint64_t pm_static() { return pm_static_at(g_base); }

static bool has_lp(uint64_t pm) {
    return ok(pm) && ok(rd64(pm + OFF_PM_LOCAL_PLAYER));
}

static uint64_t player_manager() {
    // TypeInfo RVA is on libil2cpp; also try g_base (unity) for original path
    static uint64_t il2 = 0;
    if (!il2) {
        il2 = find_lib("libil2cpp.so");
        if (!il2) il2 = find_lib("libil2cpp");
    }
    uint64_t c0 = pm_static_at(g_base);
    uint64_t c1 = pm_lazy_at(g_base);
    uint64_t c2 = il2 ? pm_static_at(il2) : 0;
    uint64_t c3 = il2 ? pm_lazy_at(il2) : 0;
    if (has_lp(c0)) return c0;
    if (has_lp(c1)) return c1;
    if (has_lp(c2)) return c2;
    if (has_lp(c3)) return c3;
    if (ok(c0)) return c0;
    if (ok(c1)) return c1;
    if (ok(c2)) return c2;
    return c3;
}

static Vector3 player_pos(uint64_t p) {
    Vector3 r{};
    if (!ok(p)) return r;
    uint64_t mc = rd64(p + OFF_PLAYER_MOVEMENT_CTRL);
    if (!ok(mc)) return r;
    uint64_t td = rd64(mc + OFF_MC_TRANSFORM_DATA);
    if (!ok(td)) return r;
    rdv(td + OFF_TD_POSITION, r);
    return r;
}

struct TM { float pos[4], rot[4], scl[4]; };

static bool tp(uint64_t tr, Vector3& o, Quaternion* qr = nullptr) {
    if (!ok(tr)) return false;
    uint64_t n = rd64(tr + 0x10);
    if (!ok(n)) return false;
    uint64_t m = rd64(n + OFF_TRANSFORM_MATRIX);
    if (!ok(m)) return false;
    int32_t idx = rd32(n + OFF_TRANSFORM_INDEX);
    if (idx < 0 || idx > 100000) return false;
    uint64_t ml = rd64(m + OFF_MATRIX_LIST);
    if (!ok(ml)) return false;
    uint64_t mi = rd64(m + OFF_MATRIX_INDICES);
    if (!ok(mi)) return false;
    TM b;
    if (!rdb(ml + (uint64_t)idx * TRANSFORM_MATRIX_SIZE, &b, sizeof(b))) return false;
    float gx = b.pos[0], gy = b.pos[1], gz = b.pos[2];
    float qx = b.rot[0], qy = b.rot[1], qz = b.rot[2], qw = b.rot[3];
    if (ok(mi)) {
        int32_t pi = 0;
        if (!rdb(mi + (uint64_t)idx * 4, &pi, sizeof(pi))) goto done;
        int lp = 0;
        while (pi >= 0 && lp < 100) {
            lp++;
            TM p;
            if (!rdb(ml + (uint64_t)pi * TRANSFORM_MATRIX_SIZE, &p, sizeof(p))) break;
            float px = p.pos[0], py = p.pos[1], pz = p.pos[2];
            float prx = p.rot[0], pry = p.rot[1], prz = p.rot[2], prw = p.rot[3];
            float sx = p.scl[0], sy = p.scl[1], sz = p.scl[2];
            float scx = gx * sx, scy = gy * sy, scz = gz * sz;
            float qx2 = prx * 2.f, qy2 = pry * 2.f, qz2 = prz * 2.f;
            float xx = prx * qx2, yy = pry * qy2, zz = prz * qz2;
            float xy = prx * qy2, xz = prx * qz2, yz = pry * qz2;
            float wx = prw * qx2, wy = prw * qy2, wz = prw * qz2;
            gx = (1.f - (yy + zz)) * scx + (xy - wz) * scy + (xz + wy) * scz + px;
            gy = (xy + wz) * scx + (1.f - (xx + zz)) * scy + (yz - wx) * scz + py;
            gz = (xz - wy) * scx + (yz + wx) * scy + (1.f - (xx + yy)) * scz + pz;
            float nqx = prw * qx + prx * qw + pry * qz - prz * qy;
            float nqy = prw * qy - prx * qz + pry * qw + prz * qx;
            float nqz = prw * qz + prx * qy - pry * qx + prz * qw;
            float nqw = prw * qw - prx * qx - pry * qy - prz * qz;
            qx = nqx; qy = nqy; qz = nqz; qw = nqw;
            if (!rdb(mi + (uint64_t)pi * 4, &pi, sizeof(pi))) break;
        }
    }
done:
    o.x = gx; o.y = gy; o.z = gz;
    if (qr) { qr->x = qx; qr->y = qy; qr->z = qz; qr->w = qw; }
    return sane_world_pos(o);
}

struct SK { Vector3 b[BIPED_BONE_COUNT]; Quaternion q[BIPED_BONE_COUNT]; bool v[BIPED_BONE_COUNT]; bool ok; };

static uint64_t bm(uint64_t p) {
    uint64_t v = rd64(p + OFF_PLAYER_CHAR_VIEW);
    if (!ok(v)) v = rd64(p + OFF_PLAYER_VIEW_1);
    if (!ok(v)) v = rd64(p + OFF_PLAYER_VIEW_2);
    if (!ok(v)) return 0;
    uint64_t m = rd64(v + OFF_CHAR_VIEW_BIPED_MAP);
    if (!ok(m)) m = rd64(v + OFF_VIEW_BIPED_MAP);
    return ok(m) ? m : 0;
}

static int bmdbg = 0;
static uint64_t bm2(uint64_t p) {
    static const uint64_t vo[] = { 0x48, 0x50, 0x58, 0x40, 0x38, 0x60, 0x68, 0x70, 0x78, 0x30, 0x28, 0x20, 0x80, 0x88, 0x98, 0xA0 };
    static const uint64_t bo[] = { 0x48, 0x50, 0x58, 0x40, 0x38, 0x28, 0x60, 0x68, 0x70, 0x78, 0x20, 0x30, 0x18, 0x10, 0x08 };
    for (int i = 0; i < 16; i++) {
        uint64_t v = rd64(p + vo[i]);
        if (!ok(v)) continue;
        for (int j = 0; j < 15; j++) {
            uint64_t m = rd64(v + bo[j]);
            if (!ok(m)) continue;
            uint64_t b0 = rd64(m + OFF_BIPED_START);
            Vector3 t0{};
            if (!ok(b0) || !tp(b0, t0)) continue;
            float mag = fabsf(t0.x) + fabsf(t0.y) + fabsf(t0.z);
            if (mag < 0.5f) continue;
            return m;
        }
    }
    return bm(p);
}

static bool gsb(uint64_t p, SK& s) {
    memset(&s, 0, sizeof(s));
    if (!p || p < 0x10000) return false;
    uint64_t m = bm2(p);
    if (!ok(m)) return false;
    bool hh = false;
    for (int i = 0; i < BIPED_BONE_COUNT; i++) {
        uint64_t bp = rd64(m + OFF_BIPED_START + (uint64_t)(i * OFF_BIPED_STRIDE));
        if (!ok(bp)) continue;
        s.v[i] = tp(bp, s.b[i], &s.q[i]);
        if (s.v[i] && (i == BONE_HEAD || i == BONE_NECK)) hh = true;
    }
    s.ok = true;
    return hh;
}

static void sch(SK& s, float px, float py, float pz) {
    if (!s.v[BONE_HIP]) return;
    float hx = s.b[BONE_HIP].x, hy = s.b[BONE_HIP].y, hz = s.b[BONE_HIP].z;
    float dx = fabsf(hx - px), dy = fabsf(hy - py), dz = fabsf(hz - pz);
    if (dx <= 0.23f && dz <= 0.23f && dy <= 1.1f) return;
    float ox = px - hx, oy = py - hy + 0.9f, oz = pz - hz;
    for (int i = 0; i < BIPED_BONE_COUNT; i++)
        if (s.v[i]) { s.b[i].x += ox; s.b[i].y += oy; s.b[i].z += oz; }
}

static MethodInfo* lu_mi;
static void* lu_mp;
static MethodInfo* up_mi;
static void* up_mp;
static std::atomic<bool> g_tps_ok{false};
static std::atomic<bool> g_tps_hf_started{false};
static std::atomic<bool> g_body_ok{false};

// Wintex/Melodium body path: call SetTPSView (set_tps) on the game thread.
// Primary bind = Melodium libunity RVAs; name/MethodInfo is fallback only.
using pc_void_fn = void (*)(void*);
using pc_view_fn = void (*)(void*, int);
using go_set_active_fn = void (*)(void*, bool);
using rend_set_enabled_fn = void (*)(void*, bool);
static pc_void_fn fn_set_tps = nullptr;
static pc_void_fn fn_set_fps = nullptr;
static pc_void_fn fn_set_visible = nullptr;
static pc_view_fn fn_set_view_mode = nullptr;
static go_set_active_fn fn_go_set_active = nullptr;
static rend_set_enabled_fn fn_rend_set_enabled = nullptr;
static void* mi_go_set_active = nullptr;
static void* mi_rend_set_enabled = nullptr;
static bool g_unity_helpers_tried = false;
static bool g_view_rva_tried = false;

static void third_person_cam_restore(); // defined with TPS cam helpers below

static void* mp(const void* m) {
    if (!m || !ok((uint64_t)m)) return nullptr;
    // Try common MethodInfo::methodPointer slots (0x0 and 0x8)
    uintptr_t p8 = (uintptr_t)rd64((uint64_t)m + 0x8);
    if (p8 > 0x100000 && readable(p8, 4)) return (void*)p8;
    uintptr_t p0 = (uintptr_t)rd64((uint64_t)m + 0x0);
    if (p0 > 0x100000 && readable(p0, 4)) return (void*)p0;
    return nullptr;
}

// Melodium-style: accept only readable non-zero A64 code (reject our hook stub).
static bool looks_like_a64(void* p) {
    if (!p || !readable((uint64_t)p, 8)) return false;
    uint32_t w0 = *(uint32_t*)p;
    if (w0 == 0 || w0 == 0xFFFFFFFFu) return false;
    // Reject our own inline-hook stub (LDR X16, literal / BR X16)
    if (w0 == 0x58000050u) return false;
    return true;
}

static void* bind_game_rva(uint64_t rva) {
    if (!rva) return nullptr;
    uintptr_t bases[2] = { (uintptr_t)g_base, (uintptr_t)g_il2 };
    for (uintptr_t mod : bases) {
        if (!mod) continue;
        void* p = (void*)(mod + rva);
        if (!looks_like_a64(p)) continue;
        return p;
    }
    return nullptr;
}

static void resolve_view_rvas() {
    if (g_view_rva_tried && fn_set_tps) return;
    g_view_rva_tried = true;
    if (!fn_set_tps) fn_set_tps = (pc_void_fn)bind_game_rva(RVA_PC_SET_TPS);
    if (!fn_set_fps) fn_set_fps = (pc_void_fn)bind_game_rva(RVA_PC_SET_FPS);
    if (!fn_set_visible) fn_set_visible = (pc_void_fn)bind_game_rva(RVA_PC_SET_VISIBLE);
    g_body_ok.store(fn_set_tps != nullptr);
}

static Il2CppClass* unity_class(const char* ns, const char* name) {
    if (!il2cpp::domain_get || !il2cpp::domain_assembly_open ||
        !il2cpp::assembly_get_image || !il2cpp::class_from_name) return nullptr;
    Il2CppDomain* d = il2cpp::domain_get();
    if (!d) return nullptr;
    const char* asms[] = {"UnityEngine.CoreModule", "UnityEngine", "Assembly-CSharp"};
    for (const char* an : asms) {
        Il2CppAssembly* a = il2cpp::domain_assembly_open(d, an);
        if (!a) continue;
        Il2CppImage* img = il2cpp::assembly_get_image(a);
        if (!img) continue;
        Il2CppClass* c = il2cpp::class_from_name(img, ns, name);
        if (c) return c;
    }
    return nullptr;
}

static void resolve_unity_helpers() {
    if (g_unity_helpers_tried) return;
    g_unity_helpers_tried = true;
    if (!il2cpp::class_get_method_from_name) return;
    Il2CppClass* go = unity_class("UnityEngine", "GameObject");
    if (go) {
        mi_go_set_active = (void*)il2cpp::class_get_method_from_name(go, "SetActive", 1);
        if (mi_go_set_active)
            fn_go_set_active = (go_set_active_fn)mp(mi_go_set_active);
    }
    Il2CppClass* rend = unity_class("UnityEngine", "Renderer");
    if (rend) {
        mi_rend_set_enabled = (void*)il2cpp::class_get_method_from_name(rend, "set_enabled", 1);
        if (mi_rend_set_enabled)
            fn_rend_set_enabled = (rend_set_enabled_fn)mp(mi_rend_set_enabled);
    }
}

static void set_go_active(void* go, bool on) {
    if (!go || !ok((uint64_t)go) || !fn_go_set_active) return;
    fn_go_set_active(go, on);
}

static void set_renderer_enabled(void* rend, bool on) {
    if (!rend || !ok((uint64_t)rend) || !fn_rend_set_enabled) return;
    fn_rend_set_enabled(rend, on);
}

static Il2CppClass* player_controller_class() {
    if(!il2cpp::domain_get||!il2cpp::domain_assembly_open||!il2cpp::assembly_get_image||!il2cpp::class_from_name)return nullptr;
    Il2CppDomain* d=il2cpp::domain_get();
    if(!d)return nullptr;
    Il2CppAssembly* asm_=il2cpp::domain_assembly_open(d,"Assembly-CSharp");
    if(!asm_)return nullptr;
    Il2CppImage* gi=il2cpp::assembly_get_image(asm_);
    if(!gi)return nullptr;
    return il2cpp::class_from_name(gi,"Axlebolt.Standoff.Player","PlayerController");
}

// Match the obfuscated view helpers by name so we never call a guessed address.
static void resolve_view_fns(Il2CppClass* pc) {
    resolve_view_rvas();
    if (fn_set_tps && fn_set_fps && fn_set_visible && fn_set_view_mode) return;
    if (!pc || !il2cpp::class_get_methods || !il2cpp::method_get_name) return;
    void* it = nullptr;
    while (const Il2CppMethod* m = il2cpp::class_get_methods(pc, &it)) {
        const char* nm = il2cpp::method_get_name(m);
        if (!nm) continue;
        void* p = mp(m);
        if (!p) continue;
        if (!fn_set_tps && strcmp(nm, NAME_PC_SET_TPS) == 0) fn_set_tps = (pc_void_fn)p;
        else if (!fn_set_fps && strcmp(nm, NAME_PC_SET_FPS) == 0) fn_set_fps = (pc_void_fn)p;
        else if (!fn_set_visible && strcmp(nm, NAME_PC_SET_VISIBLE) == 0) fn_set_visible = (pc_void_fn)p;
        else if (!fn_set_view_mode && strcmp(nm, NAME_PC_SET_VIEW_MODE) == 0) fn_set_view_mode = (pc_view_fn)p;
    }
    g_body_ok.store(fn_set_tps != nullptr);
}

static void resolve_lu(){
    resolve_view_rvas();
    if(lu_mi && up_mi && fn_set_tps)return;
    if(!il2cpp::class_get_method_from_name) {
        resolve_view_rvas();
        return;
    }
    Il2CppClass* pc=player_controller_class();
    if(!pc){
        resolve_view_rvas();
        return;
    }
    if(!lu_mi){
        lu_mi=(MethodInfo*)il2cpp::class_get_method_from_name(pc,"LateUpdate",0);
        if(!lu_mi)lu_mi=(MethodInfo*)find_method(pc,"LateUpdate");
    }
    if(!up_mi){
        up_mi=(MethodInfo*)il2cpp::class_get_method_from_name(pc,"Update",0);
        if(!up_mi)up_mi=(MethodInfo*)find_method(pc,"Update");
    }
    resolve_view_fns(pc);
}

// Game thread only (LateUpdate/Update).
static void apply_body_view(void* player, bool tps_on) {
    if (!player || !ok((uint64_t)player)) return;
    resolve_view_rvas();
    uint64_t lp = (uint64_t)player;

    if (tps_on) {
        wr8(lp + OFF_PLAYER_VIEW_MODE, 2);
        wr8(lp + OFF_PLAYER_CHAR_VISIBLE, 1);
        if (fn_set_view_mode) fn_set_view_mode(player, 2);
        if (fn_set_tps) fn_set_tps(player);
        if (fn_set_visible) fn_set_visible(player);
        uint64_t arms_lod = rd64(lp + OFF_PLAYER_ARMS_LOD);
        if (ok(arms_lod)) wr8(arms_lod + OFF_LOD_RENDER_ENABLED, 0);
        uint64_t char_lod = rd64(lp + OFF_PLAYER_CHAR_LOD);
        if (ok(char_lod)) wr8(char_lod + OFF_LOD_RENDER_ENABLED, 1);
        uint64_t skin_lod = rd64(lp + OFF_PLAYER_SKIN_LOD);
        if (ok(skin_lod)) wr8(skin_lod + OFF_LOD_RENDER_ENABLED, 1);
    } else {
        wr8(lp + OFF_PLAYER_VIEW_MODE, 1);
        if (fn_set_view_mode) fn_set_view_mode(player, 1);
        if (fn_set_fps) fn_set_fps(player);
        uint64_t arms_lod = rd64(lp + OFF_PLAYER_ARMS_LOD);
        if (ok(arms_lod)) wr8(arms_lod + OFF_LOD_RENDER_ENABLED, 1);
        third_person_cam_restore();
    }
}

static inline float norm_yaw(float yaw) {
    if (!std::isfinite(yaw)) return 0.f;
    yaw = fmodf(yaw + 180.f, 360.f);
    if (yaw < 0.f) yaw += 360.f;
    return yaw - 180.f;
}
static inline float clamp_pitch(float p, float m = 70.f) {
    if (p > m) p = m;
    if (p < -m) p = -m;
    return p;
}

// Resolve native TransformAccess matrix entry.
// Tries ESP layout (0x28/0x30) first, then wintex (0x38/0x40).
static uint64_t native_tm_entry_ex(uint64_t transform, uint64_t mat_off, uint64_t idx_off) {
    if (!ok(transform)) return 0;
    uint64_t native = rd64(transform + 0x10);
    if (!ok(native)) return 0;
    uint64_t matrix = rd64(native + mat_off);
    if (!ok(matrix)) return 0;
    int32_t index = rd32(native + idx_off);
    if (index < 0 || index > 100000) return 0;
    uint64_t list = rd64(matrix + OFF_MATRIX_LIST);
    if (!ok(list)) return 0;
    uint64_t entry = list + (uint64_t)index * (uint64_t)TRANSFORM_MATRIX_SIZE;
    return readable(entry, TRANSFORM_MATRIX_SIZE) ? entry : 0;
}

static uint64_t native_tm_entry(uint64_t transform) {
    uint64_t e = native_tm_entry_ex(transform, OFF_NATIVE_TR_MATRIX, OFF_NATIVE_TR_INDEX);
    if (e) return e;
    return native_tm_entry_ex(transform, OFF_NATIVE_TR_MATRIX_ALT, OFF_NATIVE_TR_INDEX_ALT);
}

static bool write_tm_local_z(uint64_t transform, float z) {
    uint64_t entry = native_tm_entry(transform);
    if (!entry) return false;
    wrf(entry + 8, z);
    return true;
}

// Camera TransformAccess uses wintex layout (0x38/0x40) — ESP bone layout first
// can hit a wrong slot and freeze the view.
static bool write_tm_local_z_cam(uint64_t transform, float z) {
    uint64_t entry = native_tm_entry_ex(transform, OFF_NATIVE_TR_MATRIX_ALT, OFF_NATIVE_TR_INDEX_ALT);
    if (!entry)
        entry = native_tm_entry_ex(transform, OFF_NATIVE_TR_MATRIX, OFF_NATIVE_TR_INDEX);
    if (!entry) return false;
    wrf(entry + 8, z);
    return true;
}

// Wintex third person: only pull camera Transform local Z.
// Do NOT rewrite the live view-matrix translation — that freezes the camera in world space.
static bool third_person_cam() {
    if (!opt_tps) {
        g_tps_ok.store(false);
        return false;
    }
    uint64_t pm = player_manager();
    if (!ok(pm)) return false;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return false;

    uint64_t main_cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (!ok(main_cam)) return false;
    uint64_t cam_tr = rd64(main_cam + OFF_MAINCAM_TRANSFORM); // PlayerMainCamera+0x38
    bool wrote = false;
    if (ok(cam_tr) && write_tm_local_z_cam(cam_tr, -tps_dist))
        wrote = true;

    // Fallback: main camera holder only (same rig axis).
    if (!wrote) {
        uint64_t holder = rd64(lp + OFF_PLAYER_CAM_HOLDER);
        if (ok(holder) && write_tm_local_z_cam(holder, -tps_dist))
            wrote = true;
    }

    g_tps_ok.store(wrote);
    return wrote;
}

static void third_person_cam_restore() {
    uint64_t pm = player_manager();
    if (!ok(pm)) return;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return;
    uint64_t main_cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (ok(main_cam)) {
        uint64_t cam_tr = rd64(main_cam + OFF_MAINCAM_TRANSFORM);
        if (ok(cam_tr)) write_tm_local_z_cam(cam_tr, 0.f);
    }
    uint64_t holder = rd64(lp + OFF_PLAYER_CAM_HOLDER);
    if (ok(holder)) write_tm_local_z_cam(holder, 0.f);
    g_tps_ok.store(false);
}

// Body/arms — wintex field path. Real view switch is apply_body_view (set_tps)
// on the game thread; this keeps arms hidden and body drawn every tick.
static void third_person_model() {
    uint64_t pm = player_manager();
    if (!ok(pm)) return;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return;
    bool tp = opt_tps;

    // Dump 0.39.2: ArmsAnimationController+0xE8 is Vector3 (not Transform*).
    // Wintex hides FP arms/weapon by parking that offset far below the world.
    uint64_t arms = rd64(lp + OFF_PLAYER_ARMS_CTRL);
    if (ok(arms) && readable(arms + OFF_ARMS_LOCAL_POS, 12)) {
        if (tp) {
            wrf(arms + OFF_ARMS_LOCAL_POS + 0, 0.f);
            wrf(arms + OFF_ARMS_LOCAL_POS + 4, -1000.f);
            wrf(arms + OFF_ARMS_LOCAL_POS + 8, 0.f);
        } else {
            float ay = rdf(arms + OFF_ARMS_LOCAL_POS + 4);
            if (ay < -100.f) {
                wrf(arms + OFF_ARMS_LOCAL_POS + 0, 0.f);
                wrf(arms + OFF_ARMS_LOCAL_POS + 4, 0.f);
                wrf(arms + OFF_ARMS_LOCAL_POS + 8, 0.f);
            }
        }
    }

    // ArmsLodGroup @ Player+0xC8 — SkinnedMeshLodGroup bool gate @ 0x20.
    uint64_t arms_lod = rd64(lp + OFF_PLAYER_ARMS_LOD);
    if (ok(arms_lod)) wr8(arms_lod + OFF_LOD_RENDER_ENABLED, tp ? 0 : 1);

    if (!tp) return;

    wr8(lp + OFF_PLAYER_VIEW_MODE, 2);
    wr8(lp + OFF_PLAYER_CHAR_VISIBLE, 1);
    uint64_t char_lod = rd64(lp + OFF_PLAYER_CHAR_LOD);
    if (ok(char_lod)) wr8(char_lod + OFF_LOD_RENDER_ENABLED, 1);
    uint64_t skin_lod = rd64(lp + OFF_PLAYER_SKIN_LOD);
    if (ok(skin_lod)) wr8(skin_lod + OFF_LOD_RENDER_ENABLED, 1);

    static const uint64_t cv_offs[] = { OFF_PLAYER_CHAR_VIEW, OFF_PLAYER_VIEW_2, OFF_PLAYER_CHAR_VIEW_TPS };
    for (uint64_t off : cv_offs) {
        uint64_t cv = rd64(lp + off);
        if (ok(cv)) wr8(cv + OFF_CHAR_VIEW_OCCLUSION, 1);
    }

    uint64_t map = bm(lp);
    if (!ok(map)) return;
    for (int i = 0; i < BIPED_BONE_COUNT; i++) {
        uint64_t bone = rd64(map + OFF_BIPED_START + (uint64_t)i * OFF_BIPED_STRIDE);
        uint64_t entry = native_tm_entry(bone);
        if (!entry) continue;
        wrf(entry + 0x20, 1.f);
        wrf(entry + 0x24, 1.f);
        wrf(entry + 0x28, 1.f);
    }
}

static void tps_tick() {
    third_person_cam();
    third_person_model();
}

static void* tps_hf_thread(void*) {
    while (true) {
        if (opt_tps && g_base) tps_tick();
        usleep(400);
    }
    return nullptr;
}

static void ensure_tps_hf() {
    bool expected = false;
    if (!g_tps_hf_started.compare_exchange_strong(expected, true)) return;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    if (pthread_create(&th, &attr, tps_hf_thread, nullptr) != 0)
        g_tps_hf_started.store(false);
    pthread_attr_destroy(&attr);
}

// ---- Anti-aim: fake angles only while the game's Update runs ----
struct Euler { float pitch, yaw, roll; };
static Euler aa_real{};
static uint64_t aa_data = 0;

static float aa_fake_yaw(float real_yaw) {
    float yaw = real_yaw;
    switch (opt_aa_yaw) {
        case 1: yaw = norm_yaw(real_yaw + 165.f); break;
        case 2: {
            static float ang = 0.f, rad = 0.f;
            ang += 8.f;
            rad += 0.5f;
            if (ang >= 360.f) ang = rad = 0.f;
            yaw = norm_yaw(ang + sinf(rad) * 180.f);
            break;
        }
        case 3: yaw = norm_yaw((float)(rand() % 360)); break;
        default: break;
    }
    if (opt_aa_spin != 0.f) {
        static float spin = 0.f;
        spin += opt_aa_spin;
        if (spin >= 360.f) spin -= 360.f;
        yaw = norm_yaw(spin);
    }
    if (opt_aa_chaos) yaw = norm_yaw((float)(rand() % 360));
    if (opt_aa_jitter && opt_aa_spin == 0.f) {
        static int frames = 0;
        static bool flip = false;
        int need = opt_aa_frames > 0 ? opt_aa_frames : 1;
        if (frames >= need) { frames = 0; flip = !flip; }
        ++frames;
        yaw = norm_yaw(yaw + (flip ? (float)opt_aa_range : (float)-opt_aa_range));
    }
    return yaw;
}

// Write fake angles right before the game's Update reads them.
static void aa_begin(void* local) {
    aa_data = 0;
    if (!opt_aa || !local) return;
    uint64_t aim = rd64((uint64_t)local + OFF_PLAYER_AIM);
    if (!ok(aim)) return;
    uint64_t ad = rd64(aim + OFF_AIM_AIMING_DATA);
    if (!ok(ad)) return;

    float pitch = rdf(ad + OFF_AIMDATA_CUR_AIM);
    float yaw = rdf(ad + OFF_AIMDATA_CUR_EULER + 4);
    if (!std::isfinite(pitch) || !std::isfinite(yaw)) return;
    if (pitch < -720.f || pitch > 720.f || yaw < -720.f || yaw > 720.f) return;

    aa_real = {pitch, yaw, 0.f};
    aa_data = ad;

    float fake_pitch = pitch;
    switch (opt_aa_pitch) {
        case 1: fake_pitch = -89.f; break;
        case 2: fake_pitch = 89.f; break;
        default: break;
    }
    if (opt_aa_yaw == 3 || opt_aa_chaos)
        fake_pitch = (float)(rand() % 179 - 89);

    wrf(ad + OFF_AIMDATA_CUR_AIM, clamp_pitch(fake_pitch, 89.f));
    wrf(ad + OFF_AIMDATA_CUR_EULER + 4, aa_fake_yaw(yaw));
}

// Put the player's own angles back so the next frame reads real values
// and the local view is unaffected.
static void aa_end() {
    if (!aa_data) return;
    wrf(aa_data + OFF_AIMDATA_CUR_AIM, aa_real.pitch);
    wrf(aa_data + OFF_AIMDATA_CUR_EULER + 4, aa_real.yaw);
    aa_data = 0;
}

static bool is_local_player(void* p) {
    if (!p || !ok((uint64_t)p)) return false;
    uint64_t pm = player_manager();
    return ok(pm) && (uint64_t)p == rd64(pm + OFF_PM_LOCAL_PLAYER);
}

static void hk_up(void* p) {
    if (!up_mp) return;
    bool local = is_local_player(p);
    // Melodium: set_tps BEFORE original Update so the game applies TPS this frame.
    if (local && opt_tps) apply_body_view(p, true);
    if (local) aa_begin(p);
    ((void(*)(void*))up_mp)(p);
    if (local) aa_end();
}

static void hk_lu(void* p){
    if (!lu_mp) return;
    bool local = is_local_player(p);
    static bool applied_tps = false;
    if (local) {
        if (opt_tps != applied_tps) {
            apply_body_view(p, opt_tps);
            applied_tps = opt_tps;
        } else if (opt_tps) {
            apply_body_view(p, true);
        }
    }
    ((void(*)(void*))lu_mp)(p);
    if (local) {
        third_person_model();
        if (opt_tps) third_person_cam();
    }
}

// Swap MethodInfo::methodPointer; il2cpp keeps it at 0x0 or 0x8 depending on build.
static bool hook_method_ptr(MethodInfo* mi, void* hook, void** orig) {
    if (!mi || !ok((uint64_t)mi)) return false;
    uintptr_t slot0 = (uintptr_t)rd64((uint64_t)mi + 0x0);
    uintptr_t slot8 = (uintptr_t)rd64((uint64_t)mi + 0x8);
    uintptr_t a = 0;
    size_t ptr_off = 0x8;
    if (slot8 > 0x100000 && readable(slot8, 4)) { a = slot8; ptr_off = 0x8; }
    else if (slot0 > 0x100000 && readable(slot0, 4)) { a = slot0; ptr_off = 0x0; }
    if (!a) return false;
    if ((void*)a == hook) {
        if (!*orig) *orig = (void*)a;
        return true;
    }
    *orig = (void*)a;
    long pg = sysconf(_SC_PAGESIZE);
    uintptr_t page = ((uintptr_t)mi + ptr_off) & ~(uintptr_t)(pg - 1);
    if (mprotect((void*)page, (size_t)pg, PROT_READ | PROT_WRITE) != 0) return false;
    *(void**)((uintptr_t)mi + ptr_off) = hook;
    __builtin___clear_cache((char*)((uintptr_t)mi + ptr_off),
                            (char*)((uintptr_t)mi + ptr_off + sizeof(void*)));
    return true;
}

static bool lu_hooked;
static bool up_hooked;

// True once the local player is spawned in a round (not lobby/menu).
static bool in_match() {
    uint64_t pm = player_manager();
    if (!ok(pm)) return false;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return false;
    uint64_t cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (!ok(cam)) return false;
    Vector3 pos = player_pos(lp);
    return pos.x > -20000.f && pos.x < 20000.f &&
           pos.y > -20000.f && pos.y < 20000.f &&
           pos.z > -20000.f && pos.z < 20000.f &&
           !(pos.x == 0.f && pos.y == 0.f && pos.z == 0.f);
}

static void try_hook_lu(){
    if (!g_hooks_armed.load()) return;
    resolve_view_rvas();

    // Primary: Melodium/Halalium a64 inline hooks on libunity RVAs.
    // MethodInfo swap needs working il2cpp metadata — often unavailable after inject.
    if (!up_hooked) {
        void* target = bind_game_rva(RVA_PC_UPDATE);
        if (target) {
            void* tramp = nullptr;
            if (a64hook::install(target, (void*)hk_up, &tramp) && tramp) {
                up_mp = tramp;
                up_hooked = true;
            }
        }
    }
    if (!lu_hooked) {
        void* target = bind_game_rva(RVA_PC_LATEUPDATE);
        if (target) {
            void* tramp = nullptr;
            if (a64hook::install(target, (void*)hk_lu, &tramp) && tramp) {
                lu_mp = tramp;
                lu_hooked = true;
            }
        }
    }

    // Fallback: MethodInfo pointer swap if metadata APIs work.
    if ((!lu_hooked || !up_hooked) && (!lu_mi || !up_mi)) resolve_lu();
    if (!lu_hooked && lu_mi && hook_method_ptr(lu_mi, (void*)hk_lu, &lu_mp) && lu_mp)
        lu_hooked = true;
    if (!up_hooked && up_mi && hook_method_ptr(up_mi, (void*)hk_up, &up_mp) && up_mp)
        up_hooked = true;
}

// Arm + install PlayerController hooks. Call only in-match or via menu button.
static void setup_hooks_now() {
    g_hooks_armed.store(true);
    resolve_view_rvas();
    try_hook_lu();
    ensure_tps_hf();
}

// Auto-arm once spawned in a round (never in lobby — AC detect).
static void maybe_auto_hooks() {
    if (lu_hooked && up_hooked) return;
    if (!in_match()) return;
    // Only when a feature that needs game-thread hooks is on.
    if (!(opt_tps || opt_aa)) return;
    setup_hooks_now();
}

static bool view_matrix(float out[16]) {
    uint64_t pm = player_manager();
    if (!ok(pm)) return false;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return false;
    uint64_t cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (!ok(cam)) return false;
    uint64_t cc = rd64(cam + 0x20);
    if (!ok(cc)) return false;
    uint64_t ct = rd64(cc + 0x10);
    if (!ok(ct)) return false;
    if (!readable(ct + OFF_CAM_MATRIX_DATA, 64)) return false;
    memcpy(out, (void*)(ct + OFF_CAM_MATRIX_DATA), 64);
    for (int i = 0; i < 16; i++)
        if (!std::isfinite(out[i])) return false;
    float pw = out[3] * out[3] + out[7] * out[7] + out[11] * out[11];
    if (!(pw > 1e-6f)) return false;
    return true;
}

static bool w2s(const Vector3& w, const float m[16], float& sx, float& sy) {
    float X = m[0] * w.x + m[4] * w.y + m[8] * w.z + m[12];
    float Y = m[1] * w.x + m[5] * w.y + m[9] * w.z + m[13];
    float W = m[3] * w.x + m[7] * w.y + m[11] * w.z + m[15];
    if (W <= 0.0001f) return false;
    float iw = 1.f / W;
    sx = (X * iw + 1.f) * 0.5f * (float)scr_w;
    sy = (1.f - Y * iw) * 0.5f * (float)scr_h;
    return true;
}

static void skb(ImDrawList* dl, const float vm[16], const Vector3* b, const bool* v,
                const int* id, int cnt, ImU32 c) {
    ImVec2 p[8];
    int n = 0;
    for (int i = 0; i < cnt && n < 8; i++) {
        int j = id[i];
        if (!v[j]) return;
        float sx, sy;
        if (!w2s(b[j], vm, sx, sy)) return;
        p[n++] = ImVec2(sx, sy);
    }
    if (n >= 2)
        dl->AddPolyline(p, n, c, ImDrawFlags_None, 1.5f);
}

static void sk(uint64_t pl, const float vm[16]) {
    SK s;
    if (!gsb(pl, s) || !s.ok) return;
    Vector3 pp = player_pos(pl);
    sch(s, pp.x, pp.y, pp.z);
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImU32 c = IM_COL32(255, 255, 255, 255);
    static const int sp[] = { BONE_NECK, BONE_SPINE, BONE_HIP };
    static const int la[] = { BONE_NECK, BONE_LEFT_UPPERARM, BONE_LEFT_FOREARM, BONE_LEFT_HAND };
    static const int ra[] = { BONE_NECK, BONE_RIGHT_UPPERARM, BONE_RIGHT_FOREARM, BONE_RIGHT_HAND };
    static const int ll[] = { BONE_HIP, BONE_LEFT_LEG, BONE_LEFT_FOOT };
    static const int rl[] = { BONE_HIP, BONE_RIGHT_LEG, BONE_RIGHT_FOOT };
    skb(dl, vm, s.b, s.v, sp, 3, c);
    skb(dl, vm, s.b, s.v, la, 4, c);
    skb(dl, vm, s.b, s.v, ra, 4, c);
    skb(dl, vm, s.b, s.v, ll, 3, c);
    skb(dl, vm, s.b, s.v, rl, 3, c);
}

static bool valid_controller(uint64_t p, uint64_t lp) {
    return ok(p) && p != lp;
}

static int try_list(uint64_t list, int layout, uint64_t lp, uint64_t* out, int cap) {
    int count;
    uint64_t buf;
    if (layout == 0) {
        count = rd32(list + 0x20);
        buf = rd64(list + 0x18);
    } else {
        count = rd32(list + 0x18);
        buf = rd64(list + 0x10);
    }
    if (count <= 0 || count > 96) return 0;
    if (!ok(buf) || buf == list) return 0;
    int m = 0;
    for (int i = 0; i < count && m < cap; i++) {
        uint64_t off = (layout == 0) ? (OFF_LIST_ENTRY_BASE + OFF_LIST_ENTRY_STRIDE * (uint64_t)i) : ((uint64_t)i << 3);
        uint64_t e = rd64(buf + off);
        if (!valid_controller(e, lp)) continue;
        out[m++] = e;
    }
    return m;
}

static int collect_players(uint64_t pm, uint64_t lp, uint64_t* out, int cap) {
    uint64_t best[96];
    int bestn = 0;
    uint64_t tmp[96];

    auto try_keep = [&](uint64_t list, int layout) {
        int n = try_list(list, layout, lp, tmp, cap);
        if (n > bestn) {
            bestn = n;
            for (int i = 0; i < n; i++) best[i] = tmp[i];
        }
    };

    uint64_t la = rd64(pm + OFF_PM_PLAYER_LIST);
    if (ok(la)) {
        try_keep(la, 0);
        try_keep(la, 1);
    }
    uint64_t lb = rd64(pm + 0x10);
    if (ok(lb)) {
        try_keep(lb, 1);
        try_keep(lb, 0);
    }

    int m = 0;
    for (int i = 0; i < bestn && m < cap; i++) {
        bool dup = false;
        for (int j = 0; j < m; j++) if (out[j] == best[i]) { dup = true; break; }
        if (!dup) out[m++] = best[i];
    }
    return m;
}

static int draw_esp() {
    if (!scr_w || !scr_h) return 0;
    if (!opt_box && !opt_health && !opt_dist && !opt_skeleton) return 0;
    uint64_t pm = player_manager();
    if (!ok(pm)) return 0;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return 0;
    float vm[16];
    if (!view_matrix(vm)) return 0;
    Vector3 lpos = player_pos(lp);
    int lteam = rd8(lp + OFF_PLAYER_TEAM);
    uint64_t players[96];
    int np = collect_players(pm, lp, players, 96);
    if (!np) return 0;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    int drawn = 0;
    for (int i = 0; i < np; i++) {
        uint64_t e = players[i];
        if (rd8(e + OFF_PLAYER_TEAM) == lteam) continue;
        Vector3 pp = player_pos(e);
        if (!(pp.x > -20000.f && pp.x < 20000.f && pp.y > -20000.f && pp.y < 20000.f && pp.z > -20000.f && pp.z < 20000.f)) continue;
        float hp = (float)health_of(e);
        if (hp <= 0.f) continue;
        float ddx = pp.x - lpos.x, ddy = pp.y - lpos.y, ddz = pp.z - lpos.z;
        float dist = sqrtf(ddx * ddx + ddy * ddy + ddz * ddz);
        if (dist > 500.f) continue;
        Vector3 head(pp.x, pp.y + PLAYER_HEIGHT, pp.z);
        float hx, hy, bx, by;
        if (!w2s(head, vm, hx, hy)) continue;
        if (!w2s(pp, vm, bx, by)) continue;
        float y1 = fminf(hy, by), y2 = fmaxf(hy, by);
        float h = y2 - y1;
        if (h < 2.f) continue;
        float bw = h * 0.25f;
        float cx = (hx + bx) * 0.5f;
        if (opt_box)
            dl->AddRect(ImVec2(cx - bw, y1), ImVec2(cx + bw, y2), IM_COL32(255, 255, 255, 255), 0.f, 0, 1.5f);
        if (opt_health) {
            int hpi = (int)hp;
            if (hpi < 0) hpi = 0;
            if (hpi > 100) hpi = 100;
            float bhx = cx - bw - 6.f;
            float fh = h * (hpi / 100.f);
            ImU32 c = IM_COL32(0, 255, 60, 255);
            dl->AddRectFilled(ImVec2(bhx - 1.f, y1 - 1.f), ImVec2(bhx + 3.f, y2 + 1.f), IM_COL32(0, 0, 0, 200));
            dl->AddRectFilled(ImVec2(bhx, y2 - fh), ImVec2(bhx + 2.f, y2), c);
        }
        if (opt_dist) {
            char txt[24];
            snprintf(txt, sizeof(txt), "%dm", (int)dist);
            dl->AddText(ImVec2(cx - 15.f, y2 + 3.f), IM_COL32(255, 255, 255, 255), txt);
        }
        if (opt_skeleton) sk(e, vm);
        drawn++;
    }
    return drawn;
}

static void draw_watermark() {
    const char* brand = "xxxstux";
    char line[96];
    snprintf(line, sizeof(line), "t.me · 0.39.2 · tps%c body%c %c%c",
             g_tps_ok.load() ? '+' : '-',
             g_body_ok.load() ? '+' : '-',
             lu_hooked ? 'L' : '-',
             up_hooked ? 'U' : '-');

    ImGui::SetNextWindowPos(ImVec2(16.f, 16.f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##watermark", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize);

    ImVec2 brand_sz = ImGui::CalcTextSize(brand);
    ImVec2 line_sz = ImGui::CalcTextSize(line);
    float width = (brand_sz.x > line_sz.x ? brand_sz.x : line_sz.x) + 24.f;
    float height = brand_sz.y + line_sz.y + 18.f;

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 rmin = p;
    ImVec2 rmax = ImVec2(p.x + width, p.y + height);

    dl->AddRectFilled(rmin, rmax, IM_COL32(18, 18, 18, 170), 6.0f);
    dl->AddRect(rmin, rmax, IM_COL32(70, 70, 70, 180), 6.0f, 0, 1.2f);
    dl->AddRectFilled(ImVec2(rmin.x, rmin.y), ImVec2(rmin.x + 3.f, rmax.y), IM_COL32(220, 220, 220, 220), 2.0f);

    ImVec2 brand_pos = ImVec2(p.x + 12.f, p.y + 5.f);
    ImVec2 line_pos = ImVec2(p.x + 12.f, p.y + 5.f + brand_sz.y + 2.f);
    dl->AddText(ImVec2(brand_pos.x + 1, brand_pos.y + 1), IM_COL32(0, 0, 0, 160), brand);
    dl->AddText(brand_pos, IM_COL32(235, 235, 235, 255), brand);
    dl->AddText(ImVec2(line_pos.x + 1, line_pos.y + 1), IM_COL32(0, 0, 0, 140), line);
    dl->AddText(line_pos, IM_COL32(180, 180, 180, 240), line);

    ImGui::Dummy(ImVec2(width, height));
    ImGui::SetCursorScreenPos(rmin);
    if (ImGui::InvisibleButton("##wm_click", ImVec2(width, height)))
        menu_open = !menu_open;

    ImGui::End();
}

// Melodium-style checkbox (ButtonBehavior + filled mark).
static bool melo_checkbox(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);

    const float square_sz = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + 10,
                                      pos.y + label_size.y + style.FramePadding.y * 2.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id)) return false;

    bool checked = *v;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) checked = !checked;
    if (*v != checked) {
        *v = checked;
        pressed = true;
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, ImVec2(square_sz + pos.x, square_sz + pos.y));
    window->DrawList->AddRectFilledMultiColor(
        check_bb.Min, check_bb.Max,
        ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255),
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg),
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
    if (*v) {
        window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x + 1, check_bb.Min.y + 1),
                                        ImVec2(check_bb.Max.x - 1, check_bb.Max.y - 1),
                                        ImGui::GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
    }
    window->DrawList->AddRect(check_bb.Min, check_bb.Max, ImGui::GetColorU32(ImGuiCol_Border));
    if (label_size.x > 0.0f)
        ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x + 10, check_bb.Min.y + style.FramePadding.y), label);
    return pressed;
}

static void draw_menu() {
    draw_watermark();

    if (menu_open && menu_alpha < 1.f) menu_alpha += 0.05f;
    else if (!menu_open && menu_alpha > 0.f) menu_alpha -= 0.05f;
    if (menu_alpha < 0.f) menu_alpha = 0.f;
    if (menu_alpha > 1.f) menu_alpha = 1.f;
    if (menu_alpha <= 0.01f) return;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha);
    ImGui::SetNextWindowPos(ImVec2(16.f, 96.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.f, 520.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(380.f, 320.f), ImVec2(800.f, 900.f));

    if (ImGui::Begin("##xxxstux_melo", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
        for (int i = 0; i < 8; i++) {
            ImColor borderCol = ImColor(35, 35, 35, 255);
            if (i == 1 || i == 7) borderCol = ImColor(55, 55, 55, 255);
            else if (i == 0) borderCol = ImColor(0, 0, 0, 255);
            ImGui::GetWindowDrawList()->AddRect(
                ImVec2(ImGui::GetWindowPos().x + i, ImGui::GetWindowPos().y + i),
                ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - i,
                       ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - i),
                borderCol);
        }

        ImGui::Dummy(ImVec2(0, 8));
        ImGui::Text("xxxstux");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 6));

        float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        if (ImGui::BeginChild("visuals", ImVec2(half, 0), ImGuiChildFlags_Border)) {
            ImGui::Text("visuals");
            melo_checkbox("box", &opt_box);
            melo_checkbox("health", &opt_health);
            melo_checkbox("distance", &opt_dist);
            melo_checkbox("skeleton", &opt_skeleton);
            melo_checkbox("third person", &opt_tps);
            if (opt_tps)
                ImGui::SliderFloat("distance", &tps_dist, 2.f, 6.f, "%.1f");
            ImGui::Spacing();
            if (lu_hooked && up_hooked) {
                ImGui::TextDisabled("hooks ready");
            } else if (ImGui::Button("setup hooks", ImVec2(-1, 0))) {
                setup_hooks_now();
            }
            if (!lu_hooked || !up_hooked)
                ImGui::TextDisabled("press in match");
        }
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::BeginChild("antiaim", ImVec2(0, 0), ImGuiChildFlags_Border)) {
            ImGui::Text("anti aims");
            melo_checkbox("anti aims", &opt_aa);
            const char* pitch_items = "local\0up\0down\0";
            const char* yaw_items = "local\0backward\0spiral\0chaos\0";
            ImGui::Combo("pitch", &opt_aa_pitch, pitch_items);
            ImGui::Combo("yaw", &opt_aa_yaw, yaw_items);
            melo_checkbox("jitter", &opt_aa_jitter);
            if (opt_aa_jitter) {
                ImGui::SliderInt("range", &opt_aa_range, 0, 50);
                ImGui::SliderInt("frames", &opt_aa_frames, 0, 30);
            }
            ImGui::SliderFloat("spin speed", &opt_aa_spin, 0.f, 180.f, "%.0f");
            melo_checkbox("random", &opt_aa_chaos);
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

static void apply_melo_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.WindowBorderSize = 1.f;
    style.ChildBorderSize = 1.f;
    style.FrameBorderSize = 1.f;
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(15, 15);
    style.ScrollbarRounding = 0;
    style.ScrollbarSize = 20;
    style.TouchExtraPadding = ImVec2(10.f, 10.f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
}

static void render_frame() {
    static bool ready = false;
    if (!ready) {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)scr_w, (float)scr_h);
        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
        ImFontConfig fc;
        fc.FontDataOwnedByAtlas = false;
        fc.OversampleH = fc.OversampleV = 3;
        // Melodium uses ~30px verdana; keep readable on mobile
        io.Fonts->AddFontFromMemoryTTF(pixeloperator, sizeof(pixeloperator), 36.f, &fc, io.Fonts->GetGlyphRangesCyrillic());
        apply_melo_style();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ready = true;
    }
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x != (float)scr_w || io.DisplaySize.y != (float)scr_h)
        io.DisplaySize = ImVec2((float)scr_w, (float)scr_h);
    static double last_t = 0.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double now = (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
    io.DeltaTime = last_t > 0.0 ? (float)(now - last_t) : (1.f / 60.f);
    if (io.DeltaTime <= 0.f || io.DeltaTime > 1.f) io.DeltaTime = 1.f / 60.f;
    last_t = now;

    // Melodium frame order
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    handle_touch();
    maybe_auto_hooks();
    draw_menu();
    // Field writes only here — game methods are called from the LateUpdate hook.
    if (opt_tps) tps_tick();
    draw_esp();
    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, scr_w, scr_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static EGLBoolean (*orig_swap)(EGLDisplay, EGLSurface);
static void start_once();
static void try_hook_egl();

static EGLBoolean hk_swap(EGLDisplay d, EGLSurface s) {
    // Kitty may skip JNI_OnLoad when JavaVM is missing; bootstrap from the render thread.
    start_once();
    EGLint w = 0, h = 0;
    if (d && s) {
        eglQuerySurface(d, s, EGL_WIDTH, &w);
        eglQuerySurface(d, s, EGL_HEIGHT, &h);
    }
    if (w > 1 && h > 1 && eglGetCurrentContext() != EGL_NO_CONTEXT) {
        scr_w = w;
        scr_h = h;
        GLint fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::lock_guard<std::mutex> lg(g_mtx);
        render_frame();
        if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    return orig_swap ? orig_swap(d, s) : EGL_FALSE;
}

static inline void flush(void* a, size_t n) {
    __builtin___clear_cache((char*)a, (char*)a + n);
}

static inline uint32_t a64_ldr_lit(int rt, int bytes_from_pc) {
    uint32_t imm19 = (uint32_t)(bytes_from_pc >> 2) & 0x7FFFF;
    return 0x58000000u | (imm19 << 5) | (uint32_t)(rt & 31);
}

static void build_stub(uint32_t* w, uint64_t target) {
    w[0] = a64_ldr_lit(17, 8);
    w[1] = 0xD61F0220u;
    *(uint64_t*)(w + 2) = target;
}

static void* hook_tramp(void* target, size_t n) {
    void* m = mmap(nullptr, n + 16, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (m == MAP_FAILED) return nullptr;
    uint32_t* t = (uint32_t*)m;
    memcpy(t, target, n);
    t[n / 4] = a64_ldr_lit(17, 8);
    t[n / 4 + 1] = 0xD61F0220u;
    *(uint64_t*)(t + n / 4 + 2) = (uint64_t)target + n;
    flush(m, n + 16);
    if (mprotect(m, n + 16, PROT_READ | PROT_EXEC) != 0) {
        munmap(m, n + 16);
        return nullptr;
    }
    return m;
}

static void inline_hook(void* target, void* hook, void** orig) {
    long pg = sysconf(_SC_PAGESIZE);
    uintptr_t page = (uintptr_t)target & ~(uintptr_t)(pg - 1);
    *orig = hook_tramp(target, 16);
    if (!*orig) return;
    if (mprotect((void*)page, (size_t)pg * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) return;
    build_stub((uint32_t*)target, (uint64_t)hook);
    flush(target, 16);
    mprotect((void*)page, (size_t)pg * 2, PROT_READ | PROT_EXEC);
}

using input_consume_fn = int32_t (*)(void*, void*, bool, int64_t, uint32_t*, AInputEvent**);
static input_consume_fn g_old_consume = nullptr;
static std::atomic<bool> g_input_hooked{false};

using ain_get_type_fn = int32_t (*)(const AInputEvent*);
using am_get_action_fn = int32_t (*)(const AInputEvent*);
using am_get_xy_fn = float (*)(const AInputEvent*, size_t);
static ain_get_type_fn g_ain_type = nullptr;
static am_get_action_fn g_am_action = nullptr;
static am_get_xy_fn g_am_x = nullptr;
static am_get_xy_fn g_am_y = nullptr;

static bool resolve_ainput() {
    void* lib = dlopen("libandroid.so", RTLD_NOW);
    void* h = lib ? lib : RTLD_DEFAULT;
    g_ain_type = (ain_get_type_fn)dlsym(h, "AInputEvent_getType");
    g_am_action = (am_get_action_fn)dlsym(h, "AMotionEvent_getAction");
    g_am_x = (am_get_xy_fn)dlsym(h, "AMotionEvent_getX");
    g_am_y = (am_get_xy_fn)dlsym(h, "AMotionEvent_getY");
    if (!g_ain_type) g_ain_type = &AInputEvent_getType;
    if (!g_am_action) g_am_action = &AMotionEvent_getAction;
    if (!g_am_x) g_am_x = &AMotionEvent_getX;
    if (!g_am_y) g_am_y = &AMotionEvent_getY;
    return g_ain_type && g_am_action && g_am_x && g_am_y;
}

static int32_t hk_input_consume(void* thiz, void* factory, bool consumeBatches,
                                int64_t frameTime, uint32_t* outSeq, AInputEvent** outEvent) {
    int32_t status = g_old_consume
                         ? g_old_consume(thiz, factory, consumeBatches, frameTime, outSeq, outEvent)
                         : -1;
    if (status != 0 || !outEvent || !*outEvent) return status;
    if (!g_ain_type || !g_am_action || !g_am_x || !g_am_y) return status;

    AInputEvent* ev = *outEvent;
    if (g_ain_type(ev) != AINPUT_EVENT_TYPE_MOTION) return status;

    // Melodium/nova action mask
    const int32_t action = g_am_action(ev);
    const int32_t masked = action & 0xfd;
    const float x = g_am_x(ev, 0);
    const float y = g_am_y(ev, 0);
    static bool down = false;
    if (masked == 0) down = true;
    else if (masked == 1) down = false;
    feed_touch(x, y, down);
    return status;
}

static void* find_sym_in_libs(const char* name) {
    static const char* libs[] = {
        "libinput.so", "libandroid.so", "libgui.so", "libui.so",
        "libunity.so", "libmain.so", "libil2cpp.so"
    };
    for (int i = 0; i < 7; i++) {
        void* lib = dlopen(libs[i], RTLD_NOW);
        if (!lib) continue;
        void* s = dlsym(lib, name);
        if (s) return s;
    }
    return dlsym(RTLD_DEFAULT, name);
}

static void try_hook_input() {
    if (g_input_hooked.load()) return;
    if (!resolve_ainput()) return;

    static const char* kNames[] = {
        "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE",
        "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEbxPjPPNS_10InputEventE",
    };
    void* sym = nullptr;
    for (int i = 0; i < 2 && !sym; i++)
        sym = find_sym_in_libs(kNames[i]);
    if (!sym) return;

    void* tramp = nullptr;
    if (a64hook::install(sym, (void*)hk_input_consume, &tramp) && tramp) {
        g_old_consume = (input_consume_fn)tramp;
        g_input_hooked.store(true);
        return;
    }
    // fallback to local inline_hook
    void* orig = nullptr;
    inline_hook(sym, (void*)hk_input_consume, &orig);
    if (orig) {
        g_old_consume = (input_consume_fn)orig;
        g_input_hooked.store(true);
    }
}

static void try_hook_egl() {
    if (orig_swap) {
        try_hook_input();
        return;
    }
    // Prefer already-loaded EGL; avoid dlopen while linker may still hold locks (ctor path).
    void* egl = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
    if (!egl) egl = dlsym(RTLD_NEXT, "eglSwapBuffers");
    if (!egl) {
        void* eh = dlopen("libEGL.so", RTLD_NOW);
        if (eh) egl = dlsym(eh, "eglSwapBuffers");
    }
    if (egl) {
        inline_hook(egl, (void*)hk_swap, (void**)&orig_swap);
        try_hook_input();
    }
}

static void* thread_main(void*) {
    il2cpp::resolve_rva = segment_resolve_rva;

    // Wait for libunity — all field R/W (ESP/TPS) goes through g_base = libunity.
    for (int i = 0; i < 600 && !g_base; i++) {
        build_maps();
        g_base = pick_base();
        if (!g_il2) g_il2 = resolve_il2();
        if (!g_base) usleep(100000);
    }
    if (!g_base) return nullptr;
    if (!g_il2) g_il2 = resolve_il2();

    build_segs();
    il2cpp::init_api(g_base); // field R/W; PC hooks deferred until match/setup
    touch_init();
    // Do NOT hook Update/LateUpdate here — AC detects lobby hooks.
    try_hook_egl();
    try_hook_input();

    while (true) {
        sleep(1);
        build_maps();
        if (!g_il2) g_il2 = resolve_il2();
        try_hook_egl();
        try_hook_input();
        if (!touch_count_fn || !get_touch_fn) touch_init();
        // Retry PC hooks only after user/match armed them.
        if (g_hooks_armed.load()) try_hook_lu();
        uint64_t new_base = pick_base();
        uintptr_t new_il2 = resolve_il2();
        if (new_il2 && new_il2 != g_il2) {
            g_il2 = new_il2;
            touch_init();
        }
        if (new_base && new_base != g_base) {
            g_base = new_base;
            build_segs();
            il2cpp::init_api(g_base);
            lu_hooked = false;
            up_hooked = false;
            lu_mi = nullptr;
            lu_mp = nullptr;
            up_mi = nullptr;
            up_mp = nullptr;
            fn_set_tps = fn_set_fps = fn_set_visible = nullptr;
            fn_set_view_mode = nullptr;
            fn_go_set_active = nullptr;
            fn_rend_set_enabled = nullptr;
            mi_go_set_active = mi_rend_set_enabled = nullptr;
            g_unity_helpers_tried = false;
            g_view_rva_tried = false;
            g_body_ok.store(false);
            // Keep armed flag; reinstall only if still armed (user already set up).
            if (g_hooks_armed.load()) try_hook_lu();
        }
    }
    return nullptr;
}

static void start_once() {
    static std::atomic<bool> started{false};
    bool expected = false;
    if (!started.compare_exchange_strong(expected, true)) return;
    il2cpp::resolve_rva = segment_resolve_rva;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    pthread_create(&th, &attr, thread_main, nullptr);
    pthread_attr_destroy(&attr);
}

// Custom inj + KittyMemoryEx look these up by name; keep them exported and sized.
extern "C" __attribute__((visibility("default"))) void xxxstux_entry(void* base) {
    (void)base;
    try_hook_egl();
    start_once();
}

extern "C" __attribute__((visibility("default"))) void payload_entry(void* base) {
    xxxstux_entry(base);
}

extern "C" __attribute__((visibility("default"))) void EntryPoint(void) {
    try_hook_egl();
    start_once();
}

// AndKittyInjector: do NOT start threads in constructors (linker lock + --hide remap).
// Only install EGL hook so the first swap can bootstrap if JNI_OnLoad is skipped.
__attribute__((constructor))
static void privet_ctor() {
    try_hook_egl();
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* key) {
    (void)vm;
    (void)key;
    try_hook_egl();
    start_once();
    return JNI_VERSION_1_6;
}
