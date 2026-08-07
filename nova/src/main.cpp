#include "game.hpp"
#include "overlay.hpp"
#include "elf_got_hook.hpp"
#include "mem.hpp"

#include <jni.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <elf.h>

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
using eglGetProcAddress_t = void* (*)(const char*);

static eglSwapBuffers_t g_real_swap = nullptr;
static eglGetProcAddress_t g_real_getproc = nullptr;
static std::atomic<uint64_t> g_frames{0};
static std::atomic<int> g_hook_hits{0};
static GameState g_state{};

static void* touch_thread(void*) {
    sleep(3);
    DIR* d = opendir("/dev/input");
    if (!d) return nullptr;
    int fds[8], max_x[8], max_y[8], nfd = 0;
    while (dirent* e = readdir(d)) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        input_absinfo ax{}, ay{};
        int mx = 0, my = 0;
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ax) == 0) mx = ax.maximum;
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ay) == 0) my = ay.maximum;
        if (mx < 200 || my < 200) { close(fd); continue; }
        if (nfd < 8) {
            fds[nfd] = fd; max_x[nfd] = mx; max_y[nfd] = my; ++nfd;
        } else close(fd);
    }
    closedir(d);
    if (!nfd) return nullptr;

    int abs_x = 0, abs_y = 0, active = 0;
    bool down = false;
    while (true) {
        for (int i = 0; i < nfd; ++i) {
            input_event ev{};
            while (read(fds[i], &ev, sizeof(ev)) == (ssize_t)sizeof(ev)) {
                active = i;
                if (ev.type == EV_ABS) {
                    if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) abs_x = ev.value;
                    else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) abs_y = ev.value;
                    else if (ev.code == ABS_MT_TRACKING_ID) down = ev.value != -1;
                } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                    down = ev.value != 0;
                } else if (ev.type == EV_SYN) {
                    nova_feed_touch_norm(
                        abs_x / float(max_x[active] > 0 ? max_x[active] : 1),
                        abs_y / float(max_y[active] > 0 ? max_y[active] : 1),
                        down);
                }
            }
        }
        usleep(2000);
    }
    return nullptr;
}

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    auto* orig = g_real_swap;
    if (!orig) return EGL_FALSE;

    const uint64_t f = g_frames.fetch_add(1) + 1;
    if (f < 20) return orig(dpy, surf);

    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
    if (w > 1 && h > 1 && eglGetCurrentContext() != EGL_NO_CONTEXT) {
        GLint fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
        if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, 0);
        game_tick(g_state);
        nova_overlay_frame(w, h, g_state);
        if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    return orig(dpy, surf);
}

static void* hooked_getproc(const char* name) {
    void* r = g_real_getproc ? g_real_getproc(name) : nullptr;
    if (name && std::strcmp(name, "eglSwapBuffers") == 0) {
        if (r) g_real_swap = reinterpret_cast<eglSwapBuffers_t>(r);
        return reinterpret_cast<void*>(hooked_swap);
    }
    return r;
}

// Replace exact pointer matches only inside PT_LOAD RW of selected modules (not full heap).
static int patch_ptr_in_load_rw(uintptr_t base, void* symbol, void* replace) {
    Elf64_Ehdr ehdr{};
    if (!mem::read_into(base, ehdr)) return 0;
    if (std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) return 0;
    if (ehdr.e_phnum == 0 || ehdr.e_phnum > 64) return 0;
    std::vector<Elf64_Phdr> phdrs(ehdr.e_phnum);
    if (!mem::read_bytes(base + ehdr.e_phoff, phdrs.data(), phdrs.size() * sizeof(Elf64_Phdr)))
        return 0;

    int hits = 0;
    const uintptr_t sym = reinterpret_cast<uintptr_t>(symbol);
    const uintptr_t rep = reinterpret_cast<uintptr_t>(replace);
    for (const auto& ph : phdrs) {
        if (ph.p_type != PT_LOAD) continue;
        if ((ph.p_flags & PF_W) == 0) continue;
        const uintptr_t seg = base + ph.p_vaddr;
        const size_t sz = ph.p_memsz;
        if (sz == 0 || sz > 16 * 1024 * 1024) continue;
        for (uintptr_t a = seg; a + 8 <= seg + sz; a += 8) {
            if (mem::read_ptr(a) != sym) continue;
            if (mem::write_ptr(a, rep)) ++hits;
        }
    }
    return hits;
}

static int patch_cached_swap_ptrs(void* symbol, void* replace) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    int total = 0;
    while (std::getline(maps, line)) {
        uintptr_t start = 0, end = 0, off = 0;
        char perms[8]{}, path[512]{};
        if (sscanf(line.c_str(), "%lx-%lx %7s %lx %*s %*s %511s",
                   &start, &end, perms, &off, path) < 5)
            continue;
        if (off != 0) continue;
        if (std::strchr(perms, 'x') == nullptr) continue;
        if (path[0] != '/') continue;
        const std::string p(path);
        if (p.find("libunity") == std::string::npos &&
            p.find("libmain") == std::string::npos &&
            p.find("split_config") == std::string::npos)
            continue;
        total += patch_ptr_in_load_rw(start, symbol, replace);
    }
    return total;
}

static bool do_hook() {
    void* egl = dlopen("libEGL.so", RTLD_NOW);
    void* sym_swap = dlsym(egl ? egl : RTLD_DEFAULT, "eglSwapBuffers");
    void* sym_gpa = dlsym(egl ? egl : RTLD_DEFAULT, "eglGetProcAddress");
    if (!sym_swap) return false;
    g_real_swap = reinterpret_cast<eglSwapBuffers_t>(sym_swap);
    if (sym_gpa) g_real_getproc = reinterpret_cast<eglGetProcAddress_t>(sym_gpa);

    int hits = 0;
    void* slot = nullptr;

    // 1) PLT/RELA by name
    hits += elfhook::hook_symbol_name("eglSwapBuffers",
                                      reinterpret_cast<void*>(hooked_swap), &slot);
    if (sym_gpa) {
        hits += elfhook::hook_symbol_name("eglGetProcAddress",
                                          reinterpret_cast<void*>(hooked_getproc), nullptr);
    }

    // 2) Cached function pointers inside libunity PT_LOAD RW
    hits += patch_cached_swap_ptrs(sym_swap, reinterpret_cast<void*>(hooked_swap));

    g_hook_hits.store(hits);
    if (hits <= 0) return false;

    nova_set_hook_mode(1);
    return true;
}

static void* worker(void*) {
    sleep(4);

    bool ok = false;
    for (int i = 0; i < 12 && !ok; ++i) {
        ok = do_hook();
        if (!ok) sleep(1);
    }
    if (!ok) {
        nova_set_hook_mode(0);
        return nullptr;
    }

    // Re-patch once more after unity fully up (late eglGetProcAddress caches)
    sleep(3);
    do_hook();

    pthread_t th;
    pthread_create(&th, nullptr, touch_thread, nullptr);
    pthread_detach(th);
    return nullptr;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    pthread_create(&th, &attr, worker, nullptr);
    pthread_attr_destroy(&attr);
    return JNI_VERSION_1_6;
}
