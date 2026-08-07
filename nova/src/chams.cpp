#include "chams.hpp"
#include "offsets.hpp"
#include "module_base.hpp"
#include "a64_inline_hook.hpp"
#include "dobby.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>

// Halalium/Melodium chams:
//   Shader.Find → Material(shader) → set_color / _ZTest=8 / _ZWrite=0
//   Renderer.set_material on CharacterLOD.skinned_mesh (@ player+0x128 → +0x30)
// Applied from PlayerController.Update (libunity+0x8E7C40C) on Unity thread.
// CRITICAL: never allocate Material/strings every tick — that freezes the game.

namespace {

struct Color4 {
    float r, g, b, a;
};

using fn_shader_find  = void* (*)(void* name);
using fn_mat_ctor     = void (*)(void* mat, void* shader);
using fn_mat_color    = void (*)(void* mat, Color4 c);
using fn_mat_float    = void (*)(void* mat, void* name, float v);
using fn_renderer_set = void (*)(void* renderer, void* mat);

using fn_domain_get = void* (*)();
using fn_domain_asm = void* (*)(void*, const char*);
using fn_asm_image  = void* (*)(void*);
using fn_class_name = void* (*)(void*, const char*, const char*);
using fn_object_new = void* (*)(void*);
using fn_string_new = void* (*)(const char*);
using fn_thread_att = void* (*)(void*);
using fn_pc_update  = void (*)(void*);

ChamsConfig g_cfg;
std::atomic<int> g_applied{0};
std::atomic<int> g_hook_mode{0};
std::atomic<bool> g_ready{false};
const char* g_status = "boot";

uintptr_t g_il2cpp = 0;
uintptr_t g_unity  = 0;

fn_shader_find  p_shader_find = nullptr;
fn_mat_ctor     p_mat_ctor = nullptr;
fn_mat_color    p_mat_color = nullptr;
fn_mat_float    p_mat_float = nullptr;
fn_renderer_set p_renderer_set = nullptr;

fn_domain_get p_domain_get = nullptr;
fn_domain_asm p_domain_asm = nullptr;
fn_asm_image  p_asm_image = nullptr;
fn_class_name p_class_from_name = nullptr;
fn_object_new p_object_new = nullptr;
fn_string_new p_string_new = nullptr;
fn_thread_att p_thread_attach = nullptr;

void* g_unity_image = nullptr;
void* g_mat_class = nullptr;

fn_pc_update g_old_update = nullptr;

void* g_local = nullptr;
int   g_local_team = -1;

// Cached Il2Cpp strings (created once)
void* s_shader_names[5]{};
void* s_prop_ztest = nullptr;
void* s_prop_zwrite = nullptr;
void* s_prop_src = nullptr;
void* s_prop_dst = nullptr;
void* s_prop_metal = nullptr;
void* s_prop_gloss = nullptr;

// One shared material — Halalium-style reuse, not per-frame alloc
void* g_mat = nullptr;
int   g_mat_style = -1;
float g_mat_col[4]{-1, -1, -1, -1};
uint32_t g_cfg_epoch = 1;
uint32_t g_mat_epoch = 0;

bool looks_a64(const void* p) {
    if (!p || (uintptr_t)p < 0x10000) return false;
    const uint32_t w = *reinterpret_cast<const uint32_t*>(p);
    return w != 0 && w != 0xFFFFFFFFu;
}

void* bind_unity(uintptr_t rva) {
    if (!g_unity || !rva) return nullptr;
    void* p = (void*)(g_unity + rva);
    return looks_a64(p) ? p : nullptr;
}

void* bind_il2(uintptr_t rva) {
    if (!g_il2cpp || !rva) return nullptr;
    void* p = (void*)(g_il2cpp + rva);
    return looks_a64(p) ? p : nullptr;
}

void* dlsym_il2(const char* name) {
    void* h = dlopen("libil2cpp.so", RTLD_NOW);
    if (!h) return nullptr;
    return dlsym(h, name);
}

bool resolve_apis() {
    if (!g_il2cpp) g_il2cpp = mods::resolve_il2cpp();
    if (!g_unity)  g_unity  = mods::resolve_unity();
    if (!g_il2cpp || !g_unity) {
        g_status = "mod";
        return false;
    }

    p_shader_find  = (fn_shader_find)bind_unity(off::unity::kShaderFind);
    p_mat_ctor     = (fn_mat_ctor)bind_unity(off::unity::kMatCtor);
    p_mat_color    = (fn_mat_color)bind_unity(off::unity::kMatSetColor);
    p_mat_float    = (fn_mat_float)bind_unity(off::unity::kMatSetFloat);
    p_renderer_set = (fn_renderer_set)bind_unity(off::unity::kRendererSet);

    p_domain_get = (fn_domain_get)(dlsym_il2("il2cpp_domain_get")
                                   ?: bind_il2(off::api::kDomainGet));
    p_domain_asm = (fn_domain_asm)(dlsym_il2("il2cpp_domain_assembly_open")
                                   ?: bind_il2(off::api::kDomainAssemblyOpen));
    p_asm_image = (fn_asm_image)(dlsym_il2("il2cpp_assembly_get_image")
                                 ?: bind_il2(off::api::kAssemblyGetImage));
    p_class_from_name = (fn_class_name)(dlsym_il2("il2cpp_class_from_name")
                                        ?: bind_il2(off::api::kClassFromName));
    p_object_new = (fn_object_new)(dlsym_il2("il2cpp_object_new")
                                   ?: bind_il2(off::api::kObjectNew));
    p_string_new = (fn_string_new)(dlsym_il2("il2cpp_string_new")
                                   ?: bind_il2(off::api::kStringNew));
    p_thread_attach = (fn_thread_att)(dlsym_il2("il2cpp_thread_attach")
                                      ?: bind_il2(off::api::kThreadAttach));

    if (!p_shader_find || !p_mat_ctor || !p_mat_color || !p_mat_float || !p_renderer_set) {
        g_status = "uapi";
        return false;
    }
    if (!p_domain_get || !p_domain_asm || !p_asm_image || !p_class_from_name ||
        !p_object_new || !p_string_new) {
        g_status = "iapi";
        return false;
    }
    return true;
}

bool ensure_runtime() {
    if (g_unity_image && g_mat_class && s_prop_ztest) return true;
    if (!p_domain_get || !p_string_new) return false;

    void* domain = p_domain_get();
    if (!domain) return false;
    if (p_thread_attach) p_thread_attach(domain);

    if (!g_unity_image) {
        void* asmbl = p_domain_asm(domain, "UnityEngine.CoreModule");
        if (!asmbl) return false;
        g_unity_image = p_asm_image(asmbl);
        if (!g_unity_image) return false;
    }
    if (!g_mat_class) {
        g_mat_class = p_class_from_name(g_unity_image, "UnityEngine", "Material");
        if (!g_mat_class) return false;
    }

    static const char* shaders[] = {
        "Legacy Shaders/Diffuse",
        "Hidden/Internal-Colored",
        "Standard",
        "Standard",
        "Transparent/Diffuse",
    };
    for (int i = 0; i < 5; ++i) {
        if (!s_shader_names[i]) s_shader_names[i] = p_string_new(shaders[i]);
    }
    if (!s_prop_ztest)  s_prop_ztest  = p_string_new("_ZTest");
    if (!s_prop_zwrite) s_prop_zwrite = p_string_new("_ZWrite");
    if (!s_prop_src)    s_prop_src    = p_string_new("_SrcBlend");
    if (!s_prop_dst)    s_prop_dst    = p_string_new("_DstBlend");
    if (!s_prop_metal)  s_prop_metal  = p_string_new("_Metallic");
    if (!s_prop_gloss)  s_prop_gloss  = p_string_new("_Glossiness");
    return s_prop_ztest && s_shader_names[0];
}

bool color_changed(const float a[4], const float b[4]) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

void* rebuild_material() {
    if (!ensure_runtime()) {
        g_status = "rt";
        return nullptr;
    }

    int style = g_cfg.material;
    if (style < 0 || style > 4) style = 1;

    void* shader = p_shader_find(s_shader_names[style]);
    if (!shader && style != 0)
        shader = p_shader_find(s_shader_names[0]);
    if (!shader) {
        g_status = "sh";
        return nullptr;
    }

    void* mat = p_object_new(g_mat_class);
    if (!mat) {
        g_status = "new";
        return nullptr;
    }
    p_mat_ctor(mat, shader);

    if (style == 2) {
        p_mat_float(mat, s_prop_src, 1.5f);
        p_mat_float(mat, s_prop_dst, 2.0f);
        p_mat_float(mat, s_prop_metal, 0.5f);
        p_mat_float(mat, s_prop_gloss, 2.0f);
    } else if (style == 3) {
        p_mat_float(mat, s_prop_metal, 5.0f);
        p_mat_float(mat, s_prop_gloss, 5.0f);
    }

    Color4 c{
        g_cfg.color[0], g_cfg.color[1], g_cfg.color[2],
        style == 4 ? 0.5f : g_cfg.color[3]
    };
    p_mat_color(mat, c);
    // Melodium order: set material first in apply; ZTest after — keep props on mat here
    p_mat_float(mat, s_prop_ztest, 8.0f);
    p_mat_float(mat, s_prop_zwrite, 0.0f);

    g_mat = mat;
    g_mat_style = style;
    std::memcpy(g_mat_col, g_cfg.color, sizeof(g_mat_col));
    g_mat_epoch = g_cfg_epoch;
    g_status = "ok";
    return mat;
}

void* get_material() {
    if (g_mat && g_mat_epoch == g_cfg_epoch && g_mat_style == g_cfg.material &&
        !color_changed(g_mat_col, g_cfg.color))
        return g_mat;
    return rebuild_material();
}

// Direct LDR — safe on Unity thread inside PC.Update (Halalium style)
uintptr_t ldr(uintptr_t base, uintptr_t off) {
    if (!base) return 0;
    return *reinterpret_cast<uintptr_t*>(base + off);
}

uint8_t ldrb(uintptr_t base, uintptr_t off) {
    if (!base) return 0;
    return *reinterpret_cast<uint8_t*>(base + off);
}

void apply_to_player(void* player, bool gloves) {
    void* mat = get_material();
    if (!mat || !player || !p_renderer_set) return;

    const uintptr_t p = (uintptr_t)player;
    const uintptr_t lod = ldr(p, off::player::kCharacterLod);
    if (lod) {
        const uintptr_t mesh = ldr(lod, off::lod::kSkinnedMesh);
        if (mesh) {
            p_renderer_set((void*)mesh, mat);
            g_applied.fetch_add(1);
        }
    }

    if (gloves) {
        const uintptr_t arms = ldr(p, off::player::kArmsLod);
        if (arms) {
            const uintptr_t g = ldr(arms, off::lod::kSkinnedMesh);
            if (g) p_renderer_set((void*)g, mat);
        }
    }
}

// Apply set_material once per player until menu cfg changes (re-set every frame = lag)
void* g_done[48]{};
int g_done_n = 0;
uint32_t g_done_epoch = 0;

bool already_done(void* player) {
    if (g_done_epoch != g_cfg_epoch) {
        g_done_n = 0;
        g_done_epoch = g_cfg_epoch;
        return false;
    }
    for (int i = 0; i < g_done_n; ++i)
        if (g_done[i] == player) return true;
    return false;
}

void mark_done(void* player) {
    if (g_done_n >= 48) return;
    g_done[g_done_n++] = player;
}

void hk_pc_update(void* player) {
    if (g_old_update) g_old_update(player);

    // Fast path: chams off → zero work (hook stay for toggle later)
    if (!g_cfg.enabled || !player || !g_ready.load()) return;

    const uintptr_t p = (uintptr_t)player;
    const uintptr_t ph = ldr(p, off::player::kPhoton);
    if (!ph) return;

    const bool is_local = ldrb(ph, off::photon::kIsLocal) != 0;
    const int team = (int)ldrb(p, off::player::kTeam);

    if (is_local) {
        g_local = player;
        g_local_team = team;
        if (!g_cfg.local_chams) return;
        if (already_done(player)) return;
        apply_to_player(player, true);
        mark_done(player);
        return;
    }

    if (!g_local) return;
    if (g_cfg.team_check && team == g_local_team) return;
    if (already_done(player)) return;

    apply_to_player(player, false);
    mark_done(player);
}

bool hook_pc_update() {
    if (!g_unity) return false;
    void* target = (void*)(g_unity + off::unity::kPcUpdate);
    if (!looks_a64(target)) {
        g_status = "upd";
        return false;
    }

    // Halalium/Melodium: Dobby first for PC.Update
    void* orig = nullptr;
    if (DobbyHook(target, (void*)hk_pc_update, &orig) == 0 && orig) {
        g_old_update = (fn_pc_update)orig;
        g_hook_mode = 2;
        return true;
    }

    void* tramp = nullptr;
    if (a64hook::install(target, (void*)hk_pc_update, &tramp) && tramp) {
        g_old_update = (fn_pc_update)tramp;
        g_hook_mode = 1;
        return true;
    }

    g_status = "hook";
    return false;
}

} // namespace

ChamsConfig& chams_cfg() { return g_cfg; }

void chams_bump_cfg() { ++g_cfg_epoch; }

bool chams_install() {
    static std::atomic<bool> once{false};
    bool expected = false;
    if (!once.compare_exchange_strong(expected, true)) return g_ready.load();

    if (!resolve_apis()) {
        once = false;
        return false;
    }
    if (!hook_pc_update()) {
        once = false;
        return false;
    }
    g_ready = true;
    g_status = "ready";
    return true;
}

int chams_applied() { return g_applied.load(); }
int chams_hook_ok() { return g_hook_mode.load(); }
const char* chams_status() { return g_status; }
