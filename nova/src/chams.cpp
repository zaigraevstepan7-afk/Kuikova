#include "chams.hpp"
#include "offsets.hpp"
#include "mem.hpp"
#include "module_base.hpp"
#include "a64_inline_hook.hpp"
#include "dobby.h"

#include <atomic>
#include <cstring>
#include <dlfcn.h>

namespace {

struct Color4 {
    float r, g, b, a;
};

using fn_shader_find = void* (*)(void* name);
using fn_mat_ctor    = void (*)(void* mat, void* shader);
using fn_mat_color   = void (*)(void* mat, Color4 c);
using fn_mat_float   = void (*)(void* mat, void* name, float v);
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

fn_shader_find   p_shader_find = nullptr;
fn_mat_ctor      p_mat_ctor = nullptr;
fn_mat_color     p_mat_color = nullptr;
fn_mat_float     p_mat_float = nullptr;
fn_renderer_set  p_renderer_set = nullptr;

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

bool looks_a64(void* p) {
    if (!p || (uintptr_t)p < 0x10000) return false;
    uint32_t w = 0;
    if (!mem::read_into((uintptr_t)p, w)) return false;
    // reject all-zero / obvious non-code
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

bool ensure_unity_image() {
    if (g_unity_image && g_mat_class) return true;
    if (!p_domain_get || !p_domain_asm || !p_asm_image || !p_class_from_name)
        return false;

    void* domain = p_domain_get();
    if (!domain) return false;
    if (p_thread_attach) p_thread_attach(domain);

    void* asmbl = p_domain_asm(domain, "UnityEngine.CoreModule");
    if (!asmbl) return false;
    g_unity_image = p_asm_image(asmbl);
    if (!g_unity_image) return false;

    g_mat_class = p_class_from_name(g_unity_image, "UnityEngine", "Material");
    return g_mat_class != nullptr;
}

void* make_str(const char* s) {
    if (!p_string_new || !s) return nullptr;
    return p_string_new(s);
}

void* create_material(const char* shader_name) {
    if (!ensure_unity_image()) return nullptr;
    void* name = make_str(shader_name);
    if (!name) return nullptr;
    void* shader = p_shader_find(name);
    if (!shader) return nullptr;
    void* mat = p_object_new(g_mat_class);
    if (!mat) return nullptr;
    p_mat_ctor(mat, shader);
    return mat;
}

void set_float_prop(void* mat, const char* prop, float v) {
    if (!mat || !p_mat_float) return;
    void* n = make_str(prop);
    if (n) p_mat_float(mat, n, v);
}

void* build_styled_material(int style, const float col[4]) {
    void* mat = nullptr;
    switch (style) {
    case 0:
        mat = create_material("Legacy Shaders/Diffuse");
        break;
    case 1:
        mat = create_material("Hidden/Internal-Colored");
        break;
    case 2:
        mat = create_material("Standard");
        if (mat) {
            set_float_prop(mat, "_SrcBlend", 1.5f);
            set_float_prop(mat, "_DstBlend", 2.0f);
            set_float_prop(mat, "_Metallic", 0.5f);
            set_float_prop(mat, "_Glossiness", 2.0f);
        }
        break;
    case 3:
        mat = create_material("Standard");
        if (mat) {
            set_float_prop(mat, "_Metallic", 5.0f);
            set_float_prop(mat, "_Glossiness", 5.0f);
        }
        break;
    case 4:
        mat = create_material("Transparent/Diffuse");
        break;
    default:
        mat = create_material("Hidden/Internal-Colored");
        break;
    }
    if (!mat) return nullptr;

    Color4 c{col[0], col[1], col[2], style == 4 ? 0.5f : col[3]};
    p_mat_color(mat, c);
    // Halalium wallhack: Always (8), no Z write
    set_float_prop(mat, "_ZTest", 8.0f);
    set_float_prop(mat, "_ZWrite", 0.0f);
    return mat;
}

void apply_to_renderer(void* renderer, void* mat) {
    if (!renderer || !mat || !p_renderer_set) return;
    p_renderer_set(renderer, mat);
}

void apply_player_mesh(void* player, void* mat) {
    if (!player || !mat) return;
    const uintptr_t lod = mem::read_ptr((uintptr_t)player + off::player::kCharacterLod);
    if (!lod) return;
    const uintptr_t mesh = mem::read_ptr(lod + off::lod::kSkinnedMesh);
    if (!mesh) return;
    apply_to_renderer((void*)mesh, mat);
    g_applied.fetch_add(1);
}

void apply_gloves(void* player, void* mat) {
    if (!player || !mat) return;
    const uintptr_t arms = mem::read_ptr((uintptr_t)player + off::player::kArmsLod);
    if (!arms) return;
    const uintptr_t gloves = mem::read_ptr(arms + off::lod::kSkinnedMesh);
    if (!gloves) return;
    apply_to_renderer((void*)gloves, mat);
}

void apply_chams(void* player, bool is_local) {
    if (!g_cfg.enabled) return;
    void* mat = build_styled_material(g_cfg.material, g_cfg.color);
    if (!mat) {
        g_status = "mat";
        return;
    }
    apply_player_mesh(player, mat);
    if (is_local) apply_gloves(player, mat);
    g_status = "ok";
}

void hk_pc_update(void* player) {
    if (player && g_cfg.enabled && g_ready.load()) {
        const uintptr_t ph = mem::read_ptr((uintptr_t)player + off::player::kPhoton);
        const bool is_local = ph && mem::read<uint8_t>(ph + off::photon::kIsLocal, 0) != 0;
        const int team = (int)mem::read<uint8_t>((uintptr_t)player + off::player::kTeam, 0xFF);

        if (is_local) {
            g_local = player;
            g_local_team = team;
            if (g_cfg.local_chams) apply_chams(player, true);
        } else if (g_local) {
            if (!g_cfg.team_check || team != g_local_team)
                apply_chams(player, false);
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

    void* tramp = nullptr;
    if (a64hook::install(target, (void*)hk_pc_update, &tramp) && tramp) {
        g_old_update = (fn_pc_update)tramp;
        g_hook_mode = 1;
        return true;
    }

    void* orig = nullptr;
    if (DobbyHook(target, (void*)hk_pc_update, &orig) == 0 && orig) {
        g_old_update = (fn_pc_update)orig;
        g_hook_mode = 2;
        return true;
    }

    g_status = "hook";
    return false;
}

} // namespace

ChamsConfig& chams_cfg() { return g_cfg; }

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
