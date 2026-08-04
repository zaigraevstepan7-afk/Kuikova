#include "update.h"
#include "misc.h"
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include "visual.h"
#include "antiaim.h"
#include "world.h"
#include "chams.h"
#include "exploits.h"
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

    if (g.hit_chams) {
        auto a = safe_type<c_player_controller *>::field(_this, oxorany(0x70));
        if (a.has_value)
            target_t.hitted = a.value;
        LOGD("target_t.hitted -> %p", target_t.hitted);
    }

    old_strict_hit(_this, hit_data, player_hit_controller);
}


void (*old_update)(c_player_controller *player);
void new_update(c_player_controller *player)
{
    if (player) {
        c_photon_player *photon{};
        bool is_local{};

        photon = player->m_pPhoton;
        if (photon) {
            is_local = photon->m_bIsLocal;
            if (is_local)
                c_player->local = player;

            // if (!is_local)
            // {
            //     if (player->m_pCharacterView)
            //     {
            //         if (g.b_esp)
            //             player->m_pCharacterView->oclussion = true;
            //     }
            // }

            if (!is_local)
            {
                // Halalium "Through Walls" / force visibility
                if (g.b_through_walls || g.b_esp)
                {
                    player->m_bCharacterVisible = true;
                    player->set_visible();
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
                // func with local player

                c_misc->init(c_player->local);
                // Melodium hitmarkers off unless Halalium adds them later
                if (g.b_marker || g.b_dmarker || g.b_tracer)
                    c_visual->hits(c_player->local);
                c_antiaim->update();
                if (g.b_world || g.b_fog || g.b_sky || g.b_solid)
                    c_world->init(c_player->local);
                c_chams->local(c_player->local);
                if (g.weapon_chams)
                    c_chams->weapon(c_player->local);
            }

            if (c_globals->is_enemy(c_player->local, player))
                c_player->enemy = player;

            // c_chams->init(is_local, player->m_team, player);

            if (c_player->enemy) {

                // func with enemy player
                c_chams->enemy(c_player->enemy);
                static int last_damage{};

                if (g.hit_chams) {
                    static std::unordered_map<int, int> last_dmg;
                    c_transform* transform{};
                    c_characher_lod_group* lod_group{};
                    c_renderer* skinned_mesh{};
                    Vector3 pos{};
                    Quaternion rot{};
                    static int aaa;

                    transform = c_player->enemy->m_pTransform;
                    lod_group = c_player->enemy->m_pCharacterLOD;
                    if (!transform || !lod_group) return;

                    skinned_mesh = lod_group->skinned_mesh_render;
                    if (!skinned_mesh) return;

                    pos = transform->get_position();
                    rot = transform->get_rotation();

                    int prev = last_dmg[aaa];
                    bool new_hit = (target_t.damage > prev);

                    if (new_hit && target_t.fire) {
                        void* obj = c_methods->object_instantiate((void*)skinned_mesh, pos, rot);
                        if (obj) c_methods->object_destroy(obj, 3);
                    }

                    last_dmg[aaa] = target_t.damage;
                }
            }
        }
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
                if (g.b_scope)
                    c_visual->remove(c_player->local);

                if (g.b_aspect)
                {
                    void *cashed{};
                    c_camera *get_main;
                    float aspect = g.f_aspect;

                    get_main = (c_camera *)camera.get_main();
                    if (get_main)
                    {
                        cashed = *(void **)((uintptr_t)get_main + oxorany(0x10));
                        if (cashed)
                        {
                            *(float *)((uintptr_t)cashed + oxorany(0x4f0)) = aspect;
                            get_main->set_fov(59.9f);
                            if (g.update_matrix)
                                c_esp->update_matrix();
                        }
                    }
                }
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
    if (game)
    {
        c_player->game = game;
        // GameController.PlayerControls — Halalium/dump 0.39.2 @ Offsets::GameController::player_controls
        c_player->controls = *(c_player_controls **)((uintptr_t)game + Offsets::GameController::player_controls);
    } else {
        c_player->after_match();
    }
    old_game_update(game);
}

bool (*old_raycast)(void *, ray_t *, float, raycast_hit_t *, int32_t, uint8_t);
bool hook_raycast(void *scene, ray_t *ray, float max_distance, raycast_hit_t *hit, int32_t layer, uint8_t trigger)
{
    if (g.b_esp && g.b_silent && g.hitbox && layer == 1610637328 && max_distance == 1000.0f)
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

        auto cashed = *(void **)((uintptr_t)main_camera + oxorany(0x10));
        if (!cashed)
            return old_raycast(scene, ray, max_distance, hit, layer, trigger);

        if (c_offsets)
        {
            Vector3 a = main_camera->get_position();
            if (a != Vector3{})
                camera_pos = a;
            else
                return old_raycast(scene, ray, max_distance, hit, layer, trigger);
        }
        else
        {
            return old_raycast(scene, ray, max_distance, hit, layer, trigger);
        }

        for (int i{}; i < c_player->entity.size(); i++)
        {
            player = c_player->entity[i];
            if (!player)
                continue;

            if (c_globals->is_alive(player))
            {
                if (c_globals->is_enemy(local, player))
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

                    hitbox_group groups[] = {
                        {g.hitbox[1], _body, (int)(sizeof(_body) / sizeof(_body[0]))},
                        {g.hitbox[0], _head, (int)(sizeof(_head) / sizeof(_head[0]))},
                        {g.hitbox[2], _arms, (int)(sizeof(_arms) / sizeof(_arms[0]))},
                        {g.hitbox[3], _legs, (int)(sizeof(_legs) / sizeof(_legs[0]))},
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
                            if (!c_globals->is_bone_visible(camera_pos, bonepos))
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
    void *resolve_icall_unity() { return ((void *(*)(const char *))(base + c_offsets->icall))(this->function_name); }
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
// getrr bypass stays OFF until relocating destroy is proven stable on-device.

static bool hook_rva_tracked(uintptr_t rva, void *hook, void **out_orig, const char *tag)
{
    if (!base || !rva || !hook)
        return false;
    void *target = (void *)(base + rva);
    if (!maps_contains_exec((uintptr_t)target))
    {
        LOGD("hook_rva %s target %p not executable (wrong base?)", tag, target);
        return false;
    }
    if (!hhooks::install_tracked(target, hook, out_orig))
    {
        LOGD("hook_rva %s tracked install failed @%p", tag, target);
        return false;
    }
    LOGI("hook_rva %s OK rva=0x%lx target=%p (Halalium Dobby path)", tag, (unsigned long)rva, target);
    return true;
}

void update::init()
{
    for (int i = 0; i < 8 && !loadedlib(oxorany("libsigner.so")) && !loadedlib(oxorany("lib/arm64/libsigner.so")); i++)
        sleep(1);

    // Halalium resolves game base via "libunity.so" string first.
    const uintptr_t update_rva = Offsets::Method::PlayerController_Update;
    const uintptr_t late_rva = Offsets::Method::PlayerController_LateUpdate;
    auto try_bases = [&]() {
        uintptr_t candidates[2] = {
            find_module_base_rx("libunity.so"),
            find_module_base_rx("libil2cpp.so"),
        };
        for (uintptr_t cand : candidates)
        {
            if (!cand)
                continue;
            if (maps_contains_exec(cand + update_rva))
            {
                base = cand;
                LOGI("Halalium base selected %p (Update RVA executable)", (void *)base);
                return true;
            }
        }
        if (!base)
            base = resolve_il2cpp_base();
        return base != 0;
    };
    try_bases();

    ::init();

    bool hooked_pc = false;
    bool hooked_late = false;

    // Primary: Halalium absolute RVA hooks (Dobby-equivalent via tracked a64).
    hooked_pc = hook_rva_tracked(update_rva, (void *)new_update, (void **)&old_update, "PlayerController.Update");
    hooked_late = hook_rva_tracked(late_rva, (void *)new_lateupdate, (void **)&old_lateupdate, "PlayerController.LateUpdate");

    if (hooked_pc)
        LOGI("kikaium: Halalium RVA Update ON (0x%lx)", (unsigned long)update_rva);
    else
        LOGI("kikaium: Halalium RVA Update failed — falling back to VMT");

    // VMT fallback / supplement (Melodium-compatible when RVA base wrong).
    Il2CppClass *game_controller = nullptr;
    Il2CppClass *player_controller = nullptr;
    if (il2cpp_class_from_name && dll::charp)
    {
        game_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Game"), oxorany("GameController"));
        player_controller = (Il2CppClass *)il2cpp_class_from_name(dll::charp, oxorany("Axlebolt.Standoff.Player"), oxorany("PlayerController"));
        LOGD("class_from_name GameController=%p PlayerController=%p", game_controller, player_controller);
    }

    if (game_controller)
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

    if (hit_controller) {
        LOGD("hit_controller -> %p", hit_controller);
        vmt(hit_controller, oxorany("ACHHGEDAEGBBHFB"), (void *)strict_hit, (void **)&old_strict_hit);
        if (hit_controller->vtable)
            hit_controller->vtable[84].methodPtr = (Il2CppMethodPointer)strict_hit;
    }

    if (gun_controller)
    {
        vmt(gun_controller, oxorany("FEEBGAGHGGCGACA"), (void *)hook_executecommands, (void **)&old_executecommands);
        if (gun_controller->vtable)
            gun_controller->vtable[20].methodPtr = (Il2CppMethodPointer)hook_executecommands;
    }

    void *ray_delegate = (void *)(base + c_offsets->ray);
    for (int i = 0; i < 10 && ray_delegate && !*(void **)ray_delegate; i++)
        sleep(1);
    if (ray_delegate && *(void **)ray_delegate)
    {
        icall_hook(ray_delegate, oxorany("UnityEngine.PhysicsScene::Internal_Raycast_Injected(UnityEngine.PhysicsScene&,UnityEngine.Ray&,System.Single,UnityEngine.RaycastHit&,System.Int32,UnityEngine.QueryTriggerInteraction)"), hook_raycast, &old_raycast);
    }
    else
    {
        LOGD("ray icall slot empty — silent/AutoWall limited");
    }

    // getrr / Halalium_Bypass — intentionally off (needs reloc DobbyDestroy).
    if (Offsets::Hook::use_getrr_bypass)
    {
        void *onstart = (void *)(base + Offsets::Method::AntiCheat_OnStart_getrr);
        LOGI("getrr bypass requested @%p (experimental)", onstart);
    }

    g_sdk_ready.store(true, std::memory_order_release);
    LOGI("kikaium update::init done (Halalium reconstruct) rva_pc=%d rva_late=%d base=%p",
         (int)hooked_pc, (int)hooked_late, (void *)base);
}