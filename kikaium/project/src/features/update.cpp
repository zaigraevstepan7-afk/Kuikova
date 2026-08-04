#include "update.h"
#include "misc.h"
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include "visual.h"
#include "antiaim.h"
#include "world.h"
#include "chams.h"
#include "exploits.h"
#include "skins.h"
#include <imgui_internal.h>
#include "esp.h"
#include "globals.hpp"
#include <unordered_map>

void (*old_strict_hit)(void* _this, void* hit_data, void* player_hit_controller);
void strict_hit(void* _this, void* hit_data, void* player_hit_controller)
{
    LOGD("strict_hit CALL!");

    if (!_this || !hit_data || !player_hit_controller) {
        old_strict_hit(_this, hit_data, player_hit_controller);
        return;
    }

    // Melodium hit_chams - never arm (not Halalium)
    if (false && g.hit_chams) {
        auto a = safe_type<c_player_controller *>::field(_this, Offsets::PlayerManager::local_player);
        if (a.has_value)
            target_t.hitted = a.value;
        LOGD("target_t.hitted -> %p", target_t.hitted);
    }

    old_strict_hit(_this, hit_data, player_hit_controller);
}


void (*old_update)(c_player_controller *player);
void new_update(c_player_controller *player)
{
    if (!old_update)
        return;
    if (!player || !g_sdk_ready.load(std::memory_order_acquire))
    {
        old_update(player);
        return;
    }

    c_photon_player *photon{};
    bool is_local{};

    photon = player->m_pPhoton;
    if (photon) {
        is_local = photon->m_bIsLocal;
        if (is_local)
            c_player->local = player;

        if (!is_local)
        {
            if (g.b_through_walls)
            {
                player->m_bCharacterVisible = true;
                player->set_visible();
            }
            else if (g.b_esp)
            {
                player->m_bCharacterVisible = true;
            }
        }

        if (c_player->local) {
            if (g.b_third) {
                if (c_player->local->m_pPhoton) {
                    if (c_player->local->m_pPhoton->get_health() > 0) {
                        if (c_player->weapon_parameters && c_globals->holding_gun())
                            c_player->local->set_tps();
                        else
                            c_player->local->set_fps();
                    }
                }
            }

            c_misc->init(c_player->local);
            c_antiaim->update();
            if (g.b_world || g.b_apply_world)
                c_world->init(c_player->local);
            c_chams->local(c_player->local);
            if (is_local)
                c_skins->tick(player);
        }

        if (c_globals->is_enemy(c_player->local, player))
            c_player->enemy = player;

        if (c_player->enemy)
            c_chams->enemy(c_player->enemy);
    }
    c_player->collect(player);
    c_player->update();
    old_update(player);
}

void (*old_lateupdate)(c_player_controller *player);
void new_lateupdate(c_player_controller *player)
{
    if (player) {
        c_photon_player *photon{};
        bool is_local{};
        c_camera camera{};
        c_player_controller *local{};

        photon = player->m_pPhoton;
        if (photon)
        {
            is_local = photon->m_bIsLocal;
            if (is_local)
                local = player;

            if (local) {
                c_globals->updateGun();
                c_antiaim->late_update(local);
                // Melodium aspect/scope - never
                if (g.b_third && c_player->local && c_player->local->m_pTransform) {
                    if (c_player->local->m_pPhoton) {
                        if (c_player->local->m_pPhoton->get_health() > 0) {
                            c_visual->third_view(c_player->local->m_pTransform);
                        }
                    }
                }
            }
        }
    }
    old_lateupdate(player);
}

void (*old_game_update)(c_game_controller *game);
void new_game_update(c_game_controller *game)
{
    // Lobby-safe: GameController.Update runs in menu. Never touch fields until SDK ready,
    // and never crash the orig path if game/controls look invalid.
    if (!old_game_update)
        return;
    if (!game || !g_sdk_ready.load(std::memory_order_acquire))
    {
        old_game_update(game);
        return;
    }

    c_player->game = game;
    auto *ctrl = *(c_player_controls **)((uintptr_t)game + Offsets::GameController::player_controls);
    if (ctrl && c_globals->is_allocated(ctrl))
        c_player->controls = ctrl;

    old_game_update(game);
}

bool (*old_raycast)(void *, ray_t *, float, raycast_hit_t *, int32_t, uint8_t);
bool hook_raycast(void *scene, ray_t *ray, float max_distance, raycast_hit_t *hit, int32_t layer, uint8_t trigger)
{
    // Halalium Silent Aim - independent of Enable Esp
    if (g.b_silent && layer == 1610637328 && max_distance == 1000.0f)
    {
        Vector3 pos{};
        bool b_found = false;
        float best_dist = FLT_MAX;

        c_player_controller *player{};
        c_player_controller *local{};
        c_player_character_view *c_character{};
        c_biped_map *c_biped{};
        c_transform *bone{};
        c_transform *main_camera{};
        Vector3 camera_pos{};
        Vector3 bonepos{};

        local = c_player->local;
        if (!local)
            return old_raycast(scene, ray, max_distance, hit, layer, trigger);

        main_camera = local->m_pMainCameraHolder;
        if (!main_camera)
            return old_raycast(scene, ray, max_distance, hit, layer, trigger);

        Vector3 a = main_camera->get_position();
        if (a == Vector3{})
            return old_raycast(scene, ray, max_distance, hit, layer, trigger);
        camera_pos = a;

        for (int i{}; i < (int)c_player->entity.size(); i++)
        {
            player = c_player->entity[i];
            if (!player)
                continue;

            if (c_globals->is_alive(player) && c_globals->is_enemy(local, player))
            {
                c_character = player->m_pCharacterView;
                if (!c_character)
                    continue;

                c_biped = c_character->c_biped;
                if (!c_biped)
                    continue;

                c_transform *_head[] = {c_biped->head, c_biped->neck};
                c_transform *_body[] = {c_biped->spine, c_biped->spine1, c_biped->left_toe_base, c_biped->right_toe_base};
                c_transform *_arms[] = {c_biped->hip};
                c_transform *_legs[] = {c_biped->left_leg, c_biped->left_up_leg, c_biped->right_leg, c_biped->right_up_leg};

                struct hitbox_group
                {
                    bool enabled;
                    c_transform **bones;
                    int count;
                };

                const bool bone_aim = g.b_silent_bone;
                hitbox_group groups[] = {
                    {!bone_aim && g.hitbox[1], _body, (int)(sizeof(_body) / sizeof(_body[0]))},
                    {bone_aim || g.hitbox[0], _head, (int)(sizeof(_head) / sizeof(_head[0]))},
                    {!bone_aim && g.hitbox[2], _arms, (int)(sizeof(_arms) / sizeof(_arms[0]))},
                    {!bone_aim && g.hitbox[3], _legs, (int)(sizeof(_legs) / sizeof(_legs[0]))},
                };

                for (int w{}; w < 4; w++)
                {
                    if (!groups[w].enabled)
                        continue;
                    for (int j{}; j < groups[w].count; j++)
                    {
                        bone = groups[w].bones[j];
                        if (!bone)
                            continue;

                        bonepos = bone->get_position();
                        if (g.b_fov_check && !c_globals->in_fov(main_camera, camera_pos, bonepos, g.f_fov_check))
                            continue;
                        if (!g.b_autowall && !c_globals->is_bone_visible(camera_pos, bonepos))
                            continue;

                        float dist = (bonepos - camera_pos).length();
                        if (dist < best_dist)
                        {
                            best_dist = dist;
                            pos = bonepos;
                            b_found = true;
                        }
                    }
                }
            }
        }

        if (b_found)
        {
            ray->m_vecDirection = (pos - camera_pos).nnormalized();
        }
    }

    return old_raycast(scene, ray, max_distance, hit, layer, trigger);
}

struct weapon_controller_cmd
{
    bool to_fire{};
    bool to_aim{};
    bool to_reload{};
    bool to_action{};
    bool to_inspect{};
    bool to_unknown{};
};

void (*old_executecommands)(void *_this, weapon_controller_cmd commands, float duration, float time);
void hook_executecommands(void *_this, weapon_controller_cmd commands, float duration, float time)
{
    if (!_this)
    {
        old_executecommands(_this, commands, duration, time);
        return;
    }
    target_t.fire = commands.to_fire;

    if (g.b_fire && g.b_silent)
    {
        auto local = c_player->local;
        if (local && c_globals->is_alive(local))
        {
            if (g.b_fire)
                c_globals->updateTarget();

            if (target_t.b_found)
            {
                commands.to_fire = true;
                target_t.fire = true;
            }
        }
    }

    old_executecommands(_this, commands, duration, time);
}

bool loadedlib(const char *module_name)
{
    char line[512], path[PATH_MAX];
    FILE *fp = fopen(oxorany("/proc/self/maps"), "r");
    if (!fp)
        return false;

    while (fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%*x-%*x %*s %*x %*x:%*x %*u %s\n", path);
        if (strstr(path, module_name))
        {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

int vmt(Il2CppClass *clazz, const char *methodName, void *hookMethod, void **oldMethod)
{
    if (clazz != nullptr && methodName != nullptr && hookMethod != nullptr && oldMethod != nullptr)
    {
        for (int i{}; i < clazz->method_count; i++)
        {
            auto method = const_cast<MethodInfo *>(clazz->methods[i]);
            if (strcmp(method->name, methodName) == oxorany(0))
            {
                if (method->genericMethod != nullptr)
                {
                    return oxorany(-2);
                }

                *oldMethod = (void *)method->methodPointer;
                method->methodPointer = (decltype(method->methodPointer))hookMethod;
                method->virtualMethodPointer = (decltype(method->virtualMethodPointer))hookMethod;
                return oxorany(0);
            }
        }
        return oxorany(-1);
    }
    return oxorany(-3);
}

struct hook_info
{
    void *ptr_addr;
    void *hook_addr;
    void *orig_addr;
    bool is_swap_hook;
};

std::vector<hook_info *> hooked_funcs;

struct icall
{
    const char *dll_name;
    size_t dll_name_len;
    const char *function_name;
    size_t function_name_len;
    size_t flags = 0x200000000;

    icall(const char *dll, const char *function)
    {
        dll_name = dll;
        function_name = function;
        dll_name_len = strlen(dll);
        function_name_len = strlen(function);
    }

    void *resolve_icall() { return nullptr; }
    void *resolve_icall_unity()
    {
        // Non-Halalium icall RVA removed — resolve via dlsym if exported
        static void *(*resolve)(const char *) = nullptr;
        if (!resolve)
        {
            void *h = dlopen("libil2cpp.so", RTLD_NOW);
            if (h)
                resolve = (decltype(resolve))dlsym(h, "il2cpp_resolve_icall");
        }
        return resolve ? resolve(this->function_name) : nullptr;
    }
};

template <class h, class o>
void icall_hook(void *delegate_addr, const char *method_name, h hook, o orig, const char *dll = oxorany("libunity"))
{
    hook_info *info = new hook_info();
    info->ptr_addr = delegate_addr;
    info->hook_addr = (void *)hook;
    info->is_swap_hook = false;

    auto *_icall = new icall(dll, method_name);
    void *orig_addr = (!strcmp(dll, oxorany("libunity"))) ? _icall->resolve_icall_unity() : _icall->resolve_icall();

    if (orig)
        *(void **)orig = orig_addr;
    info->orig_addr = orig_addr;
    hooked_funcs.push_back(info);

    *(void **)delegate_addr = (void *)hook;
}

void vmtHook(uintptr_t object, int method, void* news, void** old) {
    void** vfunc = (void**)(*(uintptr_t*)object + 0xB8 + method * sizeof(uintptr_t) * 2);
    if (*vfunc != news) {
        *old = *vfunc;
        *vfunc = news;
    }
}

#include "includes/a64_inline_hook.h"
#include "includes/halalium_hooks.h"
#include "includes/module_base.h"

// Halalium egl_install @0x1d84cc DobbyHook list (base = libunity/libil2cpp):
//   Update      0x8E7C40C  → callback ~Halalium_Hooks (player tick)
//   secondary   0x8E0085C
//   tertiary    0x79FE5E0 (alt 0x147E970)
//   LateUpdate  0x8E7CF50  (= Update + 0xB44)
//   0x8D663EC / 0x8D2B2B0
// Bypass @0x1d90b8: DobbyDestroy all → call real OnStart@0x8B9579C → DobbyHook reinstall.
// getrr Bypass_getrr enabled via Offsets::Hook::use_getrr_bypass (Halalium 100%).

static bool hook_rva_tracked(uintptr_t module_base, uintptr_t rva, void *hook, void **out_orig, const char *tag)
{
    if (!module_base || !rva || !hook)
        return false;
    void *target = (void *)(module_base + rva);
    if (!maps_contains_exec((uintptr_t)target))
    {
        LOGD("hook_rva %s target %p not executable (wrong module?)", tag, target);
        return false;
    }
    if (!hhooks::looks_like_a64(target))
    {
        LOGD("hook_rva %s target %p not a64 prologue", tag, target);
        return false;
    }
    if (!hhooks::install_tracked(target, hook, out_orig))
    {
        LOGD("hook_rva %s tracked install failed @%p", tag, target);
        return false;
    }
    LOGI("hook_rva %s OK base=%p rva=0x%lx target=%p", tag, (void *)module_base, (unsigned long)rva, target);
    return true;
}


// ---------------------------------------------------------------------------
// Halalium egl_install secondary / tertiary / ExtraA / ExtraB
// Signatures from Capstone Pass A (tertiary_hook_cb / extraA_cb / extraB_cb).
// ---------------------------------------------------------------------------
void (*old_secondary)(void *gc);
void new_secondary_halalium(void *gc)
{
    // secondary_hook_cb @0x1d81fc - GameController sibling; passthrough
    if (old_secondary)
        old_secondary(gc);
}

// tertiary_hook_cb @0x1d8404 - float in s0, ptr x0, int w2; then BR orig
void (*old_tertiary)(void *a0, void *a1, uint32_t a2, float f0);
void new_tertiary_halalium(void *a0, void *a1, uint32_t a2, float f0)
{
    // Aim-vector rewrite lives in Halalium BSS; silent covered by raycast icall.
    if (old_tertiary)
        old_tertiary(a0, a1, a2, f0);
}

// extraA_cb @0x1d82a0 - (self, a1, hit) team gate then BR orig
void (*old_extraA)(void *a0, void *a1, void *a2);
void new_extraA_halalium(void *a0, void *a1, void *a2)
{
    if (old_extraA)
        old_extraA(a0, a1, a2);
}

// extraB_cb @0x1d83cc - call orig then optional clear *a1 (No spread path)
void (*old_extraB)(void *a0, void *a1);
void new_extraB_halalium(void *a0, void *a1)
{
    if (old_extraB)
        old_extraB(a0, a1);
    if (g.b_nospread && a1)
        *reinterpret_cast<uintptr_t *>(a1) = 0;
}

void update::init()
{
    for (int i = 0; i < 8 && !loadedlib(oxorany("libsigner.so")) && !loadedlib(oxorany("lib/arm64/libsigner.so")); i++)
        sleep(1);

    // CRITICAL split:
    // - base      = libil2cpp.so  (il2cpp API + TypeInfo)
    // - unity_base= libunity.so   (Halalium method RVAs like Update 0x8E7C40C)
    // Old code set base=libunity when RVA was executable, then ::init() resolved
    // il2cpp_domain_get = unity+offset and crashed ~3s after inject on img_to_asm.
    base = find_module_base_rx("libil2cpp.so");
    if (!base)
        base = resolve_il2cpp_base();
    unity_base = resolve_unity_base();

    if (!base)
    {
        LOGI("update::init abort: no libil2cpp base");
        return;
    }
    LOGI("update::init il2cpp=%p unity=%p", (void *)base, (void *)unity_base);

    ::init();
    if (!il2cpp_domain_get || !dll::charp)
    {
        LOGI("update::init abort: il2cpp API/image failed");
        return;
    }

    c_globals->init();

    const uintptr_t update_rva = Offsets::Method::PlayerController_Update;
    const uintptr_t late_rva = Offsets::Method::PlayerController_LateUpdate;

    bool hooked_pc = false;
    bool hooked_late = false;

    if (unity_base)
    {
        hooked_pc = hook_rva_tracked(unity_base, update_rva, (void *)new_update, (void **)&old_update, "PC.Update");
        hooked_late = hook_rva_tracked(unity_base, late_rva, (void *)new_lateupdate, (void **)&old_lateupdate, "PC.LateUpdate");

        if (Offsets::Hook::use_secondary_hooks)
        {
            hook_rva_tracked(unity_base, Offsets::Method::HookSecondary, (void *)new_secondary_halalium, (void **)&old_secondary, "Secondary");
            bool ter = hook_rva_tracked(unity_base, Offsets::Method::HookTertiary, (void *)new_tertiary_halalium, (void **)&old_tertiary, "Tertiary");
            if (!ter)
                hook_rva_tracked(unity_base, Offsets::Method::HookTertiaryAlt, (void *)new_tertiary_halalium, (void **)&old_tertiary, "TertiaryAlt");
            hook_rva_tracked(unity_base, Offsets::Method::HookExtraA, (void *)new_extraA_halalium, (void **)&old_extraA, "ExtraA");
            hook_rva_tracked(unity_base, Offsets::Method::HookExtraB, (void *)new_extraB_halalium, (void **)&old_extraB, "ExtraB");
        }
    }
    else
    {
        LOGI("unity_base missing - RVA hooks skipped (VMT only)");
    }

    LOGI("kikaium: Halalium RVA Update=%d Late=%d", (int)hooked_pc, (int)hooked_late);

    Il2CppClass *game_controller = nullptr;
    Il2CppClass *player_controller = nullptr;
    if (il2cpp_class_from_name && dll::charp)
    {
        game_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Game"), oxorany("GameController"));
        player_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Player"), oxorany("PlayerController"));
        LOGD("GameController=%p PlayerController=%p", game_controller, player_controller);
    }

    if (Offsets::Hook::use_vmt_update_hooks && game_controller)
        vmt(game_controller, oxorany("Update"), (void *)new_game_update, (void **)&old_game_update);

    if (!hooked_pc && player_controller)
        vmt(player_controller, oxorany("Update"), (void *)new_update, (void **)&old_update);
    if (!hooked_late && player_controller)
        vmt(player_controller, oxorany("LateUpdate"), (void *)new_lateupdate, (void **)&old_lateupdate);

    Il2CppClass *hit_controller = nullptr;
    Il2CppClass *gun_controller = nullptr;
    if (il2cpp_class_from_name && dll::charp)
    {
        hit_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Player.Hit"), oxorany("PlayerHitController"));
        gun_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Inventory.Gun"), oxorany("GunController"));
    }

    if (hit_controller)
        vmt(hit_controller, oxorany("ACHHGEDAEGBBHFB"), (void *)strict_hit, (void **)&old_strict_hit);
    if (gun_controller)
        vmt(gun_controller, oxorany("FEEBGAGHGGCGACA"), (void *)hook_executecommands, (void **)&old_executecommands);

    // Raycast icall: Melodium slot RVA removed. Silent uses MethodInfo Physics when available;
    // optional resolve via il2cpp_resolve_icall dlsym in icall_hook path.
    LOGD("ray icall hardcoded slot disabled (not Halalium) — silent via MethodInfo/Physics only");

    if (Offsets::Hook::use_getrr_bypass && unity_base)
    {
        bool ok = hhooks::install_getrr_bypass(unity_base);
        LOGI("getrr bypass %s", ok ? "ON" : "FAIL");
    }

    g_sdk_ready.store(true, std::memory_order_release);
    LOGI("Halalium reconstruct done; Halalium-style VMT ready");
    LOGI("kikaium update::init done pc=%d late=%d il2cpp=%p unity=%p",
         (int)hooked_pc, (int)hooked_late, (void *)base, (void *)unity_base);
}

