// Recovered architecture skeleton from libhalalium.so (static RE).
// NOT a drop-in rebuild of the full binary — documents control flow,
// hook table, globals and PlayerController field usage confirmed in asm.
// Offsets: see re/sdk/Offsets_0.39.2.h
//
// Binary: BuildID 12532fca99debbaa836dbbea6e5cceec95f5bbbb
// Brand:  t.me/lemminghack, 0.39.2 / Lemming / Halalium

#include <cstdint>
#include <pthread.h>
#include <android/log.h>

// --- Dobby (exported from the same .so) ---
extern "C" {
int DobbyHook(void* address, void* replace_call, void** origin_call);
int DobbyDestroy(void* address);
void* DobbySymbolResolver(const char* image_name, const char* symbol_name);
}

// --- Globals (page 0x279000) ---
struct HalaliumGlobals {
    // layout is sparse; only recovered slots:
    // 0x279064 menu_open
    // 0x2794f0 game_base
    // 0x2794f8 config*
    // 0x279578 orig_eglSwapBuffers
    // 0x279580 orig_input_consume
    // 0x2795a8 orig_Update
    // 0x2795b0 local_player
    // 0x2795f0 orig_LateUpdate
    // 0x27965c imgui_once
    // 0x2796d8 hooks_vector
    // 0x2796f0 orig_OnStart
};

static constexpr uintptr_t G = 0x279000;

struct TrackedHook { // size 0x18
    void* target;
    void* callback;
    void** orig_slot;
};

// --- Confirmed PlayerController fields ---
struct PlayerController_RE {
    char pad0[0x79];
    uint8_t team;                 // +0x79
    char pad1[0x88 - 0x7A];
    void* weaponry;               // +0x88
    char pad2[0xB8 - 0x90];
    void* occlusion;              // +0xB8
    char pad3[0xD8 - 0xC0];
    uint8_t visible;              // +0xD8
    char pad4[0xE8 - 0xD9];
    void* main_camera;            // +0xE8
    char pad5[0x160 - 0xF0];
    void* photon_player;          // +0x160
};

struct PhotonPlayer_RE {
    char pad[0x30];
    uint8_t IsLocal;              // +0x30
};

// --- Hook RVAs (add to game_base @0x2794f0) ---
namespace HookRVA {
    constexpr uintptr_t Update      = 0x8E7C40C;
    constexpr uintptr_t Secondary   = 0x8E0085C;
    constexpr uintptr_t Tertiary    = 0x79FE5E0;
    constexpr uintptr_t TertiaryAlt = 0x147E970;
    constexpr uintptr_t LateUpdate  = 0x8E7CF50; // Update + 0xB44
    constexpr uintptr_t ExtraA      = 0x8D663EC;
    constexpr uintptr_t ExtraB      = 0x8D2B2B0;
}

using EglSwapFn = int (*)(void* dpy, void* surface);
using UpdateFn  = void (*)(PlayerController_RE*);

static EglSwapFn  orig_egl = nullptr;
static UpdateFn   orig_update = nullptr;

// egl_callback @0x1d76f0
static int hooked_eglSwapBuffers(void* dpy, void* surface) {
    // once: ImGui_ImplOpenGL3 init when eglGetCurrentContext() != 0
    // each frame: eglQuerySurface size → ImGui NewFrame
    //   always draw ##watermark (Lemming / version)
    //   if menu_open: full tabs (Rage/Visuals/Skins/...)
    //   ##wm_click InvisibleButton toggles menu_open @0x279064
    // ImGui::Render + OpenGL3_RenderDrawData
    return orig_egl ? orig_egl(dpy, surface) : 0;
}

// Halalium_Hooks_Update @0x1d7a0c
static void hooked_Update(PlayerController_RE* player) {
    if (orig_update) orig_update(player);
    if (!player || !player->photon_player) return;

    auto* ph = reinterpret_cast<PhotonPlayer_RE*>(player->photon_player);
    if (ph->IsLocal) {
        // *local_player = player;  // @0x2795b0
        // rate-limited feature maintenance
    } else {
        player->visible = 1;
        // if team != local->team → ESP / aim gated by menu flags
    }
}

// Halalium_Bypass @0x1d90b8
static void* hooked_OnStart(void* self) {
    __android_log_print(4, "Halalium_Bypass", "enter");
    // for (auto& h : tracked) DobbyDestroy(h.target);
    // auto ret = orig_OnStart(self);
    // for (auto& h : tracked) DobbyHook(...); log "bypas hok result %d"
    return self;
}

// egl_install @0x1d84cc
static void install_hooks(uintptr_t game_base) {
    void* p = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (p) DobbyHook(p, (void*)hooked_eglSwapBuffers, (void**)&orig_egl);

    p = DobbySymbolResolver(
        "libinput.so",
        "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE");
    if (p) DobbyHook(p, /*input_cb 0x1d760c*/ nullptr, /*&orig_input*/ nullptr);

    if (!game_base) return;

    DobbyHook((void*)(game_base + HookRVA::Update),
              (void*)hooked_Update, (void**)&orig_update);
    // + Secondary, Tertiary, LateUpdate, ExtraA, ExtraB
    // each registered into tracked vector via helper 0x1d917c
}

extern "C" jint JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
    // pthread_create → worker @0x1d6afc → resolve game_base → install_hooks
    return 0x00010006; // JNI_VERSION_1_6
}
