#include "game.hpp"
#include "overlay.hpp"
#include "hook_aarch64.hpp"
#include "stealth.hpp"

#include <jni.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t g_orig_swap = nullptr;
static eglSwapBuffers_t g_real_swap = nullptr; // always absolute libEGL symbol
static std::atomic<bool> g_hooked{false};
static std::atomic<uint64_t> g_swap_calls{0};
static std::atomic<bool> g_overlay_on{false};
static std::atomic<bool> g_touch_on{false};
static GameState g_state{};

static void* touch_thread(void*) {
    // Start late — opening /dev/input during inject is unstable on some OEMs
    usleep(5 * 1000 * 1000);
    if (!g_overlay_on.load()) return nullptr;

    stealth::disguise_thread("InputReader");

    int fds[8];
    int max_x[8], max_y[8];
    int nfd = 0;

    DIR* d = opendir("/dev/input");
    if (!d) return nullptr;
    while (dirent* e = readdir(d)) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        input_absinfo ax{}, ay{};
        int mx = 0, my = 0;
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ax) == 0 && ax.maximum > 0) mx = ax.maximum;
        else if (ioctl(fd, EVIOCGABS(ABS_X), &ax) == 0 && ax.maximum > 0) mx = ax.maximum;
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ay) == 0 && ay.maximum > 0) my = ay.maximum;
        else if (ioctl(fd, EVIOCGABS(ABS_Y), &ay) == 0 && ay.maximum > 0) my = ay.maximum;

        if (mx < 100 || my < 100) { close(fd); continue; }
        if (nfd < 8) {
            fds[nfd] = fd; max_x[nfd] = mx; max_y[nfd] = my; ++nfd;
        } else close(fd);
    }
    closedir(d);
    if (!nfd) return nullptr;
    g_touch_on = true;

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
                    else if (ev.code == ABS_MT_TRACKING_ID) down = (ev.value != -1);
                } else if (ev.type == EV_KEY && (ev.code == BTN_TOUCH || ev.code == BTN_LEFT)) {
                    down = ev.value != 0;
                } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                    const float mx = static_cast<float>(max_x[active] > 0 ? max_x[active] : 1);
                    const float my = static_cast<float>(max_y[active] > 0 ? max_y[active] : 1);
                    nova_feed_touch_norm(abs_x / mx, abs_y / my, down);
                }
            }
        }
        usleep(2000);
    }
    return nullptr;
}

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    const uint64_t n = g_swap_calls.fetch_add(1, std::memory_order_relaxed) + 1;

    // Warmup: never touch GL/game for first frames (inject settle)
    auto call_orig = [&]() -> EGLBoolean {
        if (g_orig_swap) return g_orig_swap(dpy, surf);
        if (g_real_swap) return g_real_swap(dpy, surf);
        return EGL_FALSE;
    };

    if (n < 90) return call_orig(); // ~1.5s at 60fps
    g_overlay_on.store(true, std::memory_order_relaxed);

    EGLint w = 0, h = 0;
    if (!eglQuerySurface(dpy, surf, EGL_WIDTH, &w)) return call_orig();
    if (!eglQuerySurface(dpy, surf, EGL_HEIGHT, &h)) return call_orig();
    if (w <= 1 || h <= 1) return call_orig();
    if (eglGetCurrentContext() == EGL_NO_CONTEXT) return call_orig();

    GLint last_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
    if (last_fbo != 0) glBindFramebuffer(GL_FRAMEBUFFER, 0);

    game_tick(g_state);
    nova_overlay_frame(w, h, g_state);

    if (last_fbo != 0) glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);

    return call_orig();
}

static bool hook_via_got(void* sym) {
    void* orig = nullptr;
    if (!hook::install_got(sym, reinterpret_cast<void*>(hooked_swap), &orig))
        return false;
    g_orig_swap = reinterpret_cast<eglSwapBuffers_t>(orig);
    nova_set_hook_mode(1);
    return true;
}

// Fallback: scan ALL rw .so mappings for the symbol pointer (broader than unity-only)
static int patch_got_broad(void* symbol, void* replace) {
    if (!symbol || !replace) return 0;
    std::ifstream maps("/proc/self/maps");
    std::string line;
    int patched = 0;
    while (std::getline(maps, line)) {
        if (line.find("rw") == std::string::npos) continue;
        if (line.find(".so") == std::string::npos) continue;
        // Never patch our own trampolines / huge heaps
        if (line.find("libc.so") != std::string::npos) continue;
        if (line.find("libdl.so") != std::string::npos) continue;
        if (line.find("linker") != std::string::npos) continue;

        uintptr_t start = 0, end = 0;
        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) != 2) continue;
        if (end <= start || (end - start) > 4 * 1024 * 1024) continue;

        for (uintptr_t a = start; a + sizeof(void*) <= end; a += sizeof(void*)) {
            if (mem::read_ptr(a) != reinterpret_cast<uintptr_t>(symbol)) continue;
            if (mem::write_ptr(a, reinterpret_cast<uintptr_t>(replace)))
                ++patched;
        }
    }
    return patched;
}

static void try_hook_egl() {
    if (g_hooked.exchange(true)) return;

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (!egl) egl = dlopen("libEGL.so", RTLD_NOLOAD);
    void* sym = dlsym(egl ? egl : RTLD_DEFAULT, "eglSwapBuffers");
    if (!sym) {
        g_hooked = false;
        return;
    }
    g_real_swap = reinterpret_cast<eglSwapBuffers_t>(sym);

    if (hook_via_got(sym)) return;

    // Broader GOT scan
    const int n = patch_got_broad(sym, reinterpret_cast<void*>(hooked_swap));
    if (n > 0) {
        g_orig_swap = g_real_swap;
        hook::g_mode = hook::Mode::Got;
        nova_set_hook_mode(1);
        return;
    }

    // Last resort disabled (inline crashes). Leave unhooked.
    g_hooked = false;
    nova_set_hook_mode(0);
}

static void* boot_thread(void*) {
    stealth::disguise_thread("Signal Catcher");

    // Long settle after memfd inject / --hide remap
    usleep(3 * 1000 * 1000);

    for (int i = 0; i < 120; ++i) {
        if (dlsym(RTLD_DEFAULT, "eglSwapBuffers")) break;
        usleep(200 * 1000);
    }
    usleep(2 * 1000 * 1000);

    game_init();
    try_hook_egl();

    pthread_t th;
    pthread_create(&th, nullptr, touch_thread, nullptr);
    pthread_detach(th);
    return nullptr;
}

__attribute__((constructor))
static void nova_ctor() {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    pthread_create(&th, &attr, boot_thread, nullptr);
    pthread_attr_destroy(&attr);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    return JNI_VERSION_1_6;
}
