#include "game.hpp"
#include "overlay.hpp"
#include "a64_inline_hook.hpp"
#include "mem.hpp"
#include "dobby.h"

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
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <inttypes.h>

// Melodium/Halalium path: eglSwapBuffers -> ImGui menu. Prefer a64, then Dobby, then GOT.

using eglSwapBuffers_fn = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_fn g_old_swap = nullptr;
static std::atomic<bool> g_egl_hooked{false};
static std::atomic<bool> g_imgui_ok{false};
static std::atomic<uint64_t> g_frames{0};
static GameState g_state{};

// ---- Melodium-style GOT: scan ALL rw maps for exact symbol pointer ----
static bool hook_egl_got_slots(void* symbol, void* replacement) {
    if (!symbol || !replacement) return false;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;

    char line[512];
    int hooked = 0;
    while (fgets(line, sizeof(line), f)) {
        uintptr_t start = 0, end = 0;
        char perms[8]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s", &start, &end, perms) < 3)
            continue;
        if (perms[0] != 'r' || perms[1] != 'w') continue;
        if (end <= start || (end - start) > 64 * 1024 * 1024) continue;

        for (uintptr_t p = start; p + sizeof(void*) <= end; p += sizeof(void*)) {
            // Crash-safe read (Melodium uses direct *; we keep safe IO)
            const uintptr_t val = mem::read_ptr(p);
            if (val != reinterpret_cast<uintptr_t>(symbol)) continue;
            if (mem::write_ptr(p, reinterpret_cast<uintptr_t>(replacement)))
                ++hooked;
        }
    }
    fclose(f);
    return hooked > 0;
}

static void* touch_thread(void*) {
    sleep(2);
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

static EGLBoolean hook_egl_swap_buffers(EGLDisplay display, EGLSurface surface) {
    EGLint w = 0, h = 0;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    auto call_old = [&]() -> EGLBoolean {
        return g_old_swap ? g_old_swap(display, surface) : EGL_FALSE;
    };

    if (w <= 1 || h <= 1) return call_old();
    if (eglGetCurrentContext() == EGL_NO_CONTEXT) return call_old();

    const uint64_t f = g_frames.fetch_add(1) + 1;

    // Melodium: init ImGui on first good frame, draw menu every frame
    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Menu first — ESP only after overlay has been alive a bit
    if (f < 45) {
        nova_overlay_frame(w, h, g_state); // status+menu only; game may be empty
    } else {
        game_tick(g_state);
        nova_overlay_frame(w, h, g_state);
    }
    g_imgui_ok.store(true, std::memory_order_relaxed);

    if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return call_old();
}

static bool init_render_hook() {
    if (g_egl_hooked.exchange(true)) return true;

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    void* sym = egl ? dlsym(egl, "eglSwapBuffers") : nullptr;
    if (!sym) sym = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (!sym) {
        g_egl_hooked = false;
        nova_set_hook_mode(0);
        return false;
    }

    // 1) Melodium MENU CRITICAL: a64 inline
    void* tramp = nullptr;
    if (a64hook::install(sym, (void*)hook_egl_swap_buffers, &tramp) && tramp) {
        g_old_swap = (eglSwapBuffers_fn)tramp;
        nova_set_hook_mode(2); // i = inline
        return true;
    }

    // 2) Dobby (Melodium/Halalium)
    void* orig = nullptr;
    if (DobbyHook(sym, (void*)hook_egl_swap_buffers, &orig) == 0 && orig) {
        g_old_swap = (eglSwapBuffers_fn)orig;
        nova_set_hook_mode(2);
        return true;
    }

    // 3) GOT spray (Melodium fallback — Unity often keeps ptr in anon rw)
    g_old_swap = (eglSwapBuffers_fn)sym;
    if (hook_egl_got_slots(sym, (void*)hook_egl_swap_buffers)) {
        nova_set_hook_mode(1); // g = got
        return true;
    }

    g_egl_hooked = false;
    nova_set_hook_mode(0);
    return false;
}

static void* entry(void*) {
    // Short settle — Melodium hooks sooner; long delay = no menu on load screen
    sleep(2);

    for (int i = 0; i < 30; ++i) {
        if (init_render_hook()) break;
        sleep(1);
    }

    if (g_egl_hooked.load()) {
        pthread_t th;
        pthread_create(&th, nullptr, touch_thread, nullptr);
        pthread_detach(th);
    } else {
        // Keep retrying in background
        for (int i = 0; i < 60; ++i) {
            sleep(2);
            if (init_render_hook()) {
                pthread_t th;
                pthread_create(&th, nullptr, touch_thread, nullptr);
                pthread_detach(th);
                break;
            }
        }
    }
    return nullptr;
}

static void start_once() {
    static std::atomic<bool> started{false};
    bool expected = false;
    if (!started.compare_exchange_strong(expected, true)) return;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    pthread_create(&th, &attr, entry, nullptr);
    pthread_attr_destroy(&attr);
}

// Melodium: both ctor and JNI_OnLoad (AndKitty often only dlopen)
__attribute__((constructor))
static void nova_ctor() {
    start_once();
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    start_once();
    return JNI_VERSION_1_6;
}
