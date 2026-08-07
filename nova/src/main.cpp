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
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <errno.h>

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t g_orig_swap = nullptr;
static std::atomic<bool> g_hooked{false};
static std::atomic<uint64_t> g_swap_calls{0};
static GameState g_state{};

// ---- Touch: /dev/input with real ABS ranges (needed for menu clicks) ----
static void* touch_thread(void*) {
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
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ax) == 0 && ax.maximum > 0)
            mx = ax.maximum;
        else if (ioctl(fd, EVIOCGABS(ABS_X), &ax) == 0 && ax.maximum > 0)
            mx = ax.maximum;
        if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ay) == 0 && ay.maximum > 0)
            my = ay.maximum;
        else if (ioctl(fd, EVIOCGABS(ABS_Y), &ay) == 0 && ay.maximum > 0)
            my = ay.maximum;

        // Keep devices that look like touchscreens
        if (mx < 100 && my < 100) {
            close(fd);
            continue;
        }
        if (nfd < 8) {
            fds[nfd] = fd;
            max_x[nfd] = mx;
            max_y[nfd] = my;
            ++nfd;
        } else {
            close(fd);
        }
    }
    closedir(d);
    if (!nfd) return nullptr;

    int abs_x = 0, abs_y = 0;
    bool down = false;
    int active = 0; // which fd last event

    while (true) {
        for (int i = 0; i < nfd; ++i) {
            input_event ev{};
            while (read(fds[i], &ev, sizeof(ev)) == (ssize_t)sizeof(ev)) {
                active = i;
                if (ev.type == EV_ABS) {
                    if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
                        abs_x = ev.value;
                    else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
                        abs_y = ev.value;
                    else if (ev.code == ABS_MT_TRACKING_ID)
                        down = (ev.value != -1);
                } else if (ev.type == EV_KEY && (ev.code == BTN_TOUCH || ev.code == BTN_LEFT)) {
                    down = ev.value != 0;
                } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                    const float mx = static_cast<float>(max_x[active] > 0 ? max_x[active] : 1);
                    const float my = static_cast<float>(max_y[active] > 0 ? max_y[active] : 1);
                    // Normalized 0..1 — overlay scales to DisplaySize
                    nova_feed_touch_norm(abs_x / mx, abs_y / my, down);
                }
            }
        }
        usleep(1000);
    }
    return nullptr;
}

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    g_swap_calls.fetch_add(1, std::memory_order_relaxed);

    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);

    if (w > 1 && h > 1 && eglGetCurrentContext() != EGL_NO_CONTEXT) {
        // Draw onto the on-screen backbuffer (Unity often has another FBO bound)
        GLint last_fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
        if (last_fbo != 0)
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

        game_tick(g_state);
        nova_overlay_frame(w, h, g_state);

        if (last_fbo != 0)
            glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
    }

    if (g_orig_swap) return g_orig_swap(dpy, surf);
    return EGL_FALSE;
}

static void try_hook_egl() {
    if (g_hooked.exchange(true)) return;

    void* egl = dlopen(XS("libEGL.so"), RTLD_NOW);
    if (!egl) egl = dlopen(XS("libEGL.so"), RTLD_NOLOAD);
    void* sym = dlsym(egl ? egl : RTLD_DEFAULT, XS("eglSwapBuffers"));
    if (!sym) {
        g_hooked = false;
        return;
    }

    void* orig = nullptr;
    // Inline first — GOT alone often misses Unity's runtime EGL pointer
    if (!hook::install_inline(sym, reinterpret_cast<void*>(hooked_swap), &orig)) {
        // Fallback GOT
        if (!hook::install(sym, reinterpret_cast<void*>(hooked_swap), &orig)) {
            g_hooked = false;
            return;
        }
    } else {
        hook::g_mode = hook::Mode::Inline;
        // Also patch GOT so alternate call paths hit us
        stealth::patch_got(sym, reinterpret_cast<void*>(hooked_swap));
    }

    g_orig_swap = reinterpret_cast<eglSwapBuffers_t>(orig);
    nova_set_hook_mode(hook::g_mode == hook::Mode::Got ? 1 : 2);
}

static void* boot_thread(void*) {
    stealth::disguise_thread("Signal Catcher");

    for (int i = 0; i < 180; ++i) {
        if (dlsym(RTLD_DEFAULT, XS("eglSwapBuffers"))) {
            usleep(1500 * 1000);
            break;
        }
        usleep(300 * 1000);
    }

    game_init();
    try_hook_egl();

    // Touch for menu interaction
    pthread_t th;
    pthread_create(&th, nullptr, touch_thread, nullptr);
    pthread_detach(th);

    // Watchdog: if swap never fires, retry inline once
    for (int i = 0; i < 40; ++i) {
        usleep(250 * 1000);
        if (g_swap_calls.load() > 0) return nullptr;
    }
    if (g_swap_calls.load() == 0) {
        g_hooked = false;
        try_hook_egl();
    }
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
