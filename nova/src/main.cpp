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

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t g_orig_swap = nullptr;
static std::atomic<bool> g_hooked{false};
static GameState g_state{};
static std::atomic<int> g_hook_mode{0}; // 1=got 2=inline

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    // Always call original path safely even if overlay fails
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);

    if (w > 1 && h > 1) {
        game_tick(g_state);
        nova_overlay_frame(w, h, g_state);
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
    if (!hook::install(sym, reinterpret_cast<void*>(hooked_swap), &orig)) {
        g_hooked = false;
        return;
    }
    g_orig_swap = reinterpret_cast<eglSwapBuffers_t>(orig);
    g_hook_mode = (hook::g_mode == hook::Mode::Got) ? 1 : 2;
    nova_set_hook_mode(g_hook_mode);
}

static void* boot_thread(void*) {
    stealth::disguise_thread("Signal Catcher");

    // Wait for renderer / il2cpp
    for (int i = 0; i < 180; ++i) {
        void* sym = dlsym(RTLD_DEFAULT, XS("eglSwapBuffers"));
        if (sym) {
            // extra settle — inject mid-load is noisy for AC
            usleep(2 * 1000 * 1000);
            break;
        }
        usleep(400 * 1000);
    }

    game_init();
    try_hook_egl();
    return nullptr;
}

__attribute__((constructor))
static void nova_ctor() {
    // No logcat — AC scrapes log buffers
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t th;
    pthread_create(&th, &attr, boot_thread, nullptr);
    pthread_attr_destroy(&attr);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    // Silent — AndKittyInjector --hide --free may call this then unlink
    return JNI_VERSION_1_6;
}
