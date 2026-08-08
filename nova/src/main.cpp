#include "game.hpp"
#include "overlay.hpp"
#include "a64_inline_hook.hpp"
#include "mem.hpp"
#include "dobby.h"

#include <jni.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/input.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <inttypes.h>

// Melodium path: eglSwapBuffers + InputConsumer for menu taps.

using eglSwapBuffers_fn = EGLBoolean (*)(EGLDisplay, EGLSurface);
using input_consume_fn = int32_t (*)(void*, void*, bool, int64_t, uint32_t*, AInputEvent**);

static eglSwapBuffers_fn g_old_swap = nullptr;
static input_consume_fn g_old_consume = nullptr;
static std::atomic<bool> g_egl_hooked{false};
static std::atomic<uint64_t> g_frames{0};
static GameState g_state{};

using ain_get_type_fn = int32_t (*)(const AInputEvent*);
using am_get_action_fn = int32_t (*)(const AInputEvent*);
using am_get_xy_fn = float (*)(const AInputEvent*, size_t);
static ain_get_type_fn g_ain_type = nullptr;
static am_get_action_fn g_am_action = nullptr;
static am_get_xy_fn g_am_x = nullptr;
static am_get_xy_fn g_am_y = nullptr;

static bool resolve_ainput() {
    void* lib = dlopen("libandroid.so", RTLD_NOW);
    g_ain_type = (ain_get_type_fn)dlsym(lib ? lib : RTLD_DEFAULT, "AInputEvent_getType");
    g_am_action = (am_get_action_fn)dlsym(lib ? lib : RTLD_DEFAULT, "AMotionEvent_getAction");
    g_am_x = (am_get_xy_fn)dlsym(lib ? lib : RTLD_DEFAULT, "AMotionEvent_getX");
    g_am_y = (am_get_xy_fn)dlsym(lib ? lib : RTLD_DEFAULT, "AMotionEvent_getY");
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

    const int32_t action = g_am_action(ev);
    const int32_t masked = action & 0xfd;
    const float x = g_am_x(ev, 0);
    const float y = g_am_y(ev, 0);
    static bool down = false;
    if (masked == 0) down = true;
    else if (masked == 1) down = false;
    nova_feed_touch(x, y, down);
    return status;
}

static void install_input_hook() {
    if (!resolve_ainput()) return;
    void* lib = dlopen("libinput.so", RTLD_NOW);
    if (!lib) lib = dlopen("libandroid.so", RTLD_NOW);
    void* sym = lib ? dlsym(lib,
        "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE")
                    : nullptr;
    if (!sym) return;

    void* tramp = nullptr;
    if (a64hook::install(sym, (void*)hk_input_consume, &tramp) && tramp) {
        g_old_consume = (input_consume_fn)tramp;
        return;
    }
    void* orig = nullptr;
    if (DobbyHook(sym, (void*)hk_input_consume, &orig) == 0 && orig)
        g_old_consume = (input_consume_fn)orig;
}

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
            if (mem::read_ptr(p) != (uintptr_t)symbol) continue;
            if (mem::write_ptr(p, (uintptr_t)replacement)) ++hooked;
        }
    }
    fclose(f);
    return hooked > 0;
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

    g_frames.fetch_add(1);
    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    if (fbo) glBindFramebuffer(GL_FRAMEBUFFER, 0);

    game_tick(g_state);
    nova_overlay_frame(w, h, g_state);

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

    void* tramp = nullptr;
    if (a64hook::install(sym, (void*)hook_egl_swap_buffers, &tramp) && tramp) {
        g_old_swap = (eglSwapBuffers_fn)tramp;
        nova_set_hook_mode(2);
        install_input_hook();
        return true;
    }

    void* orig = nullptr;
    if (DobbyHook(sym, (void*)hook_egl_swap_buffers, &orig) == 0 && orig) {
        g_old_swap = (eglSwapBuffers_fn)orig;
        nova_set_hook_mode(2);
        install_input_hook();
        return true;
    }

    g_old_swap = (eglSwapBuffers_fn)sym;
    if (hook_egl_got_slots(sym, (void*)hook_egl_swap_buffers)) {
        nova_set_hook_mode(1);
        install_input_hook();
        return true;
    }

    g_egl_hooked = false;
    nova_set_hook_mode(0);
    return false;
}

static void* entry(void*) {
    sleep(2);
    for (int i = 0; i < 40; ++i) {
        if (init_render_hook()) break;
        sleep(1);
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

__attribute__((constructor))
static void nova_ctor() { start_once(); }

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    start_once();
    return JNI_VERSION_1_6;
}
