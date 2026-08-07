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

// CRITICAL: no __attribute__((constructor)).
// AndKittyInjector --hide remaps the .so right after load; ctor races = crash.

using eglSwapBuffers_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t g_real_swap = nullptr;
static std::atomic<uint64_t> g_frames{0};
static std::atomic<bool> g_ready{false};
static GameState g_state{};

static EGLBoolean hooked_swap(EGLDisplay dpy, EGLSurface surf) {
    // Always call through absolute libEGL symbol — never via possibly-patched GOT
    auto* orig = g_real_swap;
    if (!orig) return EGL_FALSE;

    const uint64_t f = g_frames.fetch_add(1) + 1;
    // Long passthrough warmup after late hook install
    if (f < 120 || !g_ready.load(std::memory_order_relaxed))
        return orig(dpy, surf);

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

static void* worker(void*) {
    // Wait for injector --hide / game renderer to fully settle
    sleep(8);

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    void* sym = dlsym(egl ? egl : RTLD_DEFAULT, "eglSwapBuffers");
    if (!sym) return nullptr;
    g_real_swap = reinterpret_cast<eglSwapBuffers_t>(sym);

    // ELF RELA/PLT only — no rw pointer spray
    const int n = elfhook::hook_symbol(sym, reinterpret_cast<void*>(hooked_swap));
    if (n <= 0) {
        // Could not hook — stay inert (no crash, no overlay)
        nova_set_hook_mode(0);
        return nullptr;
    }
    nova_set_hook_mode(1);
    g_ready.store(true, std::memory_order_relaxed);
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

// Optional: if injector uses --free after JNI_OnLoad, work must finish in OnLoad.
// We keep thread detached; do not use --free with this build.
