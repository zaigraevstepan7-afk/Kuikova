#include "game.hpp"
#include "overlay.hpp"
#include "hook_aarch64.hpp"

#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <dirent.h>
#include <cmath>

#define NOVA_LOG(...) __android_log_print(ANDROID_LOG_INFO, "nova", __VA_ARGS__)

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t g_orig_swap = nullptr;
static std::atomic<bool> g_hooked{false};
static GameState g_state{};

static void* touch_thread(void*) {
    // Best-effort: read multitouch from /dev/input/event* (needs root)
    DIR* d = opendir("/dev/input");
    if (!d) return nullptr;
    int fds[8];
    int nfd = 0;
    while (dirent* e = readdir(d)) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[64];
        snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd >= 0 && nfd < 8) fds[nfd++] = fd;
    }
    closedir(d);
    if (!nfd) {
        NOVA_LOG("no input devices");
        return nullptr;
    }
    NOVA_LOG("touch devices: %d", nfd);

    float x = 0, y = 0;
    bool down = false;
    int abs_x = 0, abs_y = 0;
    int max_x = 0, max_y = 0;

    while (true) {
        for (int i = 0; i < nfd; ++i) {
            input_event ev{};
            while (read(fds[i], &ev, sizeof(ev)) == sizeof(ev)) {
                if (ev.type == EV_ABS) {
                    if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) {
                        abs_x = ev.value;
                        if (ev.value > max_x) max_x = ev.value;
                    } else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) {
                        abs_y = ev.value;
                        if (ev.value > max_y) max_y = ev.value;
                    } else if (ev.code == ABS_MT_TRACKING_ID) {
                        down = (ev.value != -1);
                    }
                } else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                    down = ev.value != 0;
                } else if (ev.type == EV_SYN) {
                    // Map to screen roughly; overlay will use DisplaySize
                    const float sx = max_x > 0 ? (abs_x / static_cast<float>(max_x)) : abs_x;
                    const float sy = max_y > 0 ? (abs_y / static_cast<float>(max_y)) : abs_y;
                    // If max unknown, pass raw; overlay scales poorly — use abs as pixels if large
                    if (max_x > 1000) {
                        x = static_cast<float>(abs_x);
                        y = static_cast<float>(abs_y);
                    } else {
                        x = sx * 1080.0f;
                        y = sy * 2400.0f;
                    }
                    nova_feed_touch(x, y, down);
                }
            }
        }
        usleep(2000);
    }
    return nullptr;
}

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);

    game_tick(g_state);
    if (w > 0 && h > 0) {
        // Preserve GL state lightly
        GLint last_program = 0, last_tex = 0, last_vao = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);
        nova_overlay_frame(w, h, g_state);
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_tex);
        (void)last_vao;
    }

    if (g_orig_swap) return g_orig_swap(dpy, surf);
    return EGL_FALSE;
}

static void try_hook_egl() {
    if (g_hooked.exchange(true)) return;

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (!egl) egl = dlopen("libEGL.so", RTLD_NOLOAD);
    void* sym = dlsym(egl ? egl : RTLD_DEFAULT, "eglSwapBuffers");
    if (!sym) {
        NOVA_LOG("eglSwapBuffers not found");
        g_hooked = false;
        return;
    }
    void* orig = nullptr;
    if (!hook::install(sym, reinterpret_cast<void*>(hooked_swap), &orig)) {
        NOVA_LOG("hook install failed");
        g_hooked = false;
        return;
    }
    g_orig_swap = reinterpret_cast<eglSwapBuffers_t>(orig);
    NOVA_LOG("eglSwapBuffers hooked @ %p", sym);

    pthread_t th;
    pthread_create(&th, nullptr, touch_thread, nullptr);
    pthread_detach(th);
}

static void* boot_thread(void*) {
    NOVA_LOG("boot thread");
    // Wait for EGL / il2cpp to appear
    for (int i = 0; i < 120; ++i) {
        void* sym = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
        if (sym) break;
        usleep(500000);
    }
    usleep(1500000);
    game_init();
    try_hook_egl();
    return nullptr;
}

__attribute__((constructor))
static void nova_ctor() {
    NOVA_LOG("nova ctor");
    pthread_t th;
    pthread_create(&th, nullptr, boot_thread, nullptr);
    pthread_detach(th);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    NOVA_LOG("JNI_OnLoad");
    return JNI_VERSION_1_6;
}
