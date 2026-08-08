#include "chams.hpp"
#include "offsets.hpp"
#include "module_base.hpp"
#include "a64_inline_hook.hpp"
#include "dobby.h"

#include <atomic>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>

// Lemming / Halalium / Melodium chams (0.39.2):
//   Update hook on libunity+0x8E7C40C
//   Shader.Find(Diffuse / Internal-Colored / ...) → Material ctor
//   set_color → Renderer.set_material(CharacterLOD+0x30) → _ZTest/_ZTestMode=8, _ZWrite=0
//   Enemy: also force visible @ player+0xD8 (Halalium Through Walls bit)
// Lag rule: cache Material + Il2Cpp strings; never object_new every tick.
// Visual rule: re-set_material each Update like Melodium (game resets mats).

namespace {

struct Color4 {
    float r, g, b, a;
};

using fn_shader_find  = void* (*)(void* name);
using fn_mat_ctor     = void (*)(void* mat, void* shader);
using fn_mat_color    = void (*)(void* mat, Color4 c);
using fn_mat_float    = void (*)(void* mat, void* name, float v);
using fn_mat_int      = void (*)(void* mat, void* name, int v);
using fn_renderer_set = void (*)(void* renderer, void* mat);
using fn_set_visible  = void (*)(void* player);

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
fn_mat_int      p_mat_int = nullptr;
fn_renderer_set p_renderer_set = nullptr;
fn_set_visible  p_set_visible = nullptr;

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

void* s_shader_names[5]{};
void* s_prop_ztest = nullptr;
void* s_prop_ztest_mode = nullptr; // Halalium string
void* s_prop_zwrite = nullptr;
void* s_prop_src = nullptr;
void* s_prop_dst = nullptr;
void* s_prop_metal = nullptr;
void* s_prop_gloss = nullptr;

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

    // Melodium/Halalium Unity RVAs (libunity)
    p_shader_find  = (fn_shader_find)bind_unity(off::unity::kShaderFind);
    p_mat_ctor     = (fn_mat_ctor)bind_unity(off::unity::kMatCtor);
    p_mat_color    = (fn_mat_color)bind_unity(off::unity::kMatSetColor);
    p_mat_float    = (fn_mat_float)bind_unity(off::unity::kMatSetFloat);
    p_mat_int      = (fn_mat_int)bind_unity(off::unity::kMatSetInt);
    p_renderer_set = (fn_renderer_set)bind_unity(off::unity::kRendererSet);
    p_set_visible  = (fn_set_visible)bind_unity(off::unity::kSetVisible);

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

    // Same shader names as Lemming SO strings / Melodium chams.cpp
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
    if (!s_prop_ztest)      s_prop_ztest      = p_string_new("_ZTest");
    if (!s_prop_ztest_mode) s_prop_ztest_mode = p_string_new("_ZTestMode");
    if (!s_prop_zwrite)     s_prop_zwrite     = p_string_new("_ZWrite");
    if (!s_prop_src)        s_prop_src        = p_string_new("_SrcBlend");
    if (!s_prop_dst)        s_prop_dst        = p_string_new("_DstBlend");
    if (!s_prop_metal)      s_prop_metal      = p_string_new("_Metallic");
    if (!s_prop_gloss)      s_prop_gloss      = p_string_new("_Glossiness");
    return s_prop_ztest && s_shader_names[0] && s_shader_names[1];
}

bool color_changed(const float a[4], const float b[4]) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

void wallhack_props(void* mat) {
    if (!mat) return;
    // Melodium: set_float _ZTest=8, _ZWrite=0 after set_material
    // Halalium SO also has _ZTestMode
    if (p_mat_float) {
        if (s_prop_ztest) p_mat_float(mat, s_prop_ztest, 8.0f);
        if (s_prop_ztest_mode) p_mat_float(mat, s_prop_ztest_mode, 8.0f);
        if (s_prop_zwrite) p_mat_float(mat, s_prop_zwrite, 0.0f);
    }
    if (p_mat_int) {
        if (s_prop_ztest) p_mat_int(mat, s_prop_ztest, 8);
        if (s_prop_ztest_mode) p_mat_int(mat, s_prop_ztest_mode, 8);
        if (s_prop_zwrite) p_mat_int(mat, s_prop_zwrite, 0);
    }
}

void* rebuild_material() {
    if (!ensure_runtime()) {
        g_status = "rt";
        return nullptr;
    }

    int style = g_cfg.material;
    if (style < 0 || style > 4) style = 0; // Melodium default Diffuse

    void* shader = p_shader_find(s_shader_names[style]);
    if (!shader && style != 0)
        shader = p_shader_find(s_shader_names[0]);
    if (!shader && style != 1)
        shader = p_shader_find(s_shader_names[1]);
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
    wallhack_props(mat);

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

uintptr_t ldr(uintptr_t base, uintptr_t off) {
    if (!base) return 0;
    return *reinterpret_cast<uintptr_t*>(base + off);
}

uint8_t ldrb(uintptr_t base, uintptr_t off) {
    if (!base) return 0;
    return *reinterpret_cast<uint8_t*>(base + off);
}

void force_visible(void* player) {
    if (!player) return;
    // Halalium Update: strb #1 → [player,#0xd8]
    *reinterpret_cast<uint8_t*>((uintptr_t)player + off::player::kVisible) = 1;
    if (p_set_visible) p_set_visible(player);
}

bool apply_mesh(void* player, void* mat) {
    if (!player || !mat || !p_renderer_set) return false;
    const uintptr_t lod = ldr((uintptr_t)player, off::player::kCharacterLod);
    if (!lod) return false;
    const uintptr_t mesh = ldr(lod, off::lod::kSkinnedMesh);
    if (!mesh) return false;

    // Melodium order: set_material then ZTest props
    p_renderer_set((void*)mesh, mat);
    wallhack_props(mat);
    g_applied.fetch_add(1);
    return true;
}

void apply_gloves(void* player, void* mat) {
    if (!player || !mat || !p_renderer_set) return;
    // Melodium: m_pLOD @0x120 → gloves @0x30
    const uintptr_t arms = ldr((uintptr_t)player, off::player::kArmsLod);
    if (!arms) return;
    const uintptr_t gloves = ldr(arms, off::lod::kSkinnedMesh);
    if (gloves) p_renderer_set((void*)gloves, mat);
}

void chams_enemy(void* player) {
    void* mat = get_material();
    if (!mat) return;
    force_visible(player);
    apply_mesh(player, mat);
}

void chams_local(void* player) {
    void* mat = get_material();
    if (!mat) return;
    apply_mesh(player, mat);
    apply_gloves(player, mat);
}

void hk_pc_update(void* player) {
    // Melodium: feature work then old_update at the end
    if (player && g_cfg.enabled && g_ready.load()) {
        const uintptr_t p = (uintptr_t)player;
        const uintptr_t ph = ldr(p, off::player::kPhoton);
        if (ph) {
            const bool is_local = ldrb(ph, off::photon::kIsLocal) != 0;
            const int team = (int)ldrb(p, off::player::kTeam);

            if (is_local) {
                g_local = player;
                g_local_team = team;
                if (g_cfg.local_chams)
                    chams_local(player);
            } else if (g_local) {
                // Melodium is_enemy: team != local team
                if (!g_cfg.team_check || team != g_local_team)
                    chams_enemy(player);
            }
        }
    }

    if (g_old_update) g_old_update(player);
}

bool hook_pc_update() {
    if (!g_unity) return false;
    void* target = (void*)(g_unity + off::unity::kPcUpdate);
    if (!looks_a64(target)) {
        g_status = "upd";
        return false;
    }

    // Halalium: Dobby on PC.Update
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
