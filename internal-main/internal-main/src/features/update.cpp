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
                player->m_bCharacterVisible = true;
                player->set_visible();
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
                c_visual->hits(c_player->local);
                c_antiaim->update();
                c_world->init(c_player->local);
                c_chams->local(c_player->local);
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
        // GameController.PlayerControls @ 0x2B0 on 0.39.2 (was 0x298 / 0x2A0)
        c_player->controls = *(c_player_controls **)((uintptr_t)game + oxorany(0x2B0));
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
#include "includes/module_base.h"

// Halalium hooks PlayerController.Update by absolute RVA (Dobby), not by name.
// Feng 0.39.2 dump: Update=0x8E7C40C LateUpdate=0x8E7CF50 — matches Halalium install immediates.
// NOTE: getrr (OnStart@0x8B9579C) bypass DISABLED — inline patch crashed on inject;
// Halalium uses Dobby reloc which we don't have yet.
static bool hook_rva(uintptr_t rva, void *hook, void **out_orig, const char *tag)
{
    if (!base || !rva || !hook)
        return false;
    void *target = (void *)(base + rva);
    if (!maps_contains_exec((uintptr_t)target))
    {
        LOGD("hook_rva %s target %p not executable (wrong base?)", tag, target);
        return false;
    }
    void *tramp = nullptr;
    if (!a64hook::install(target, hook, &tramp))
    {
        LOGD("hook_rva %s a64 install failed @%p", tag, target);
        return false;
    }
    if (out_orig)
        *out_orig = tramp;
    LOGD("hook_rva %s OK rva=0x%lx target=%p tramp=%p", tag, (unsigned long)rva, target, tramp);
    return true;
}

void update::init()
{
    LOGI("update::init begin (menu-only)");

    // No libsigner wait — it only delayed us into races with the GL thread.

    LOGI("update::init resolve base...");
    auto try_bases = [&]() {
        uintptr_t candidates[2] = {
            find_module_base_rx("libil2cpp.so"),
            find_module_base_rx("libunity.so"),
        };
        for (uintptr_t cand : candidates)
        {
            if (!cand)
                continue;
            if (maps_contains_exec(cand + 0x8E7C40C))
            {
                base = cand;
                LOGI("selected game base %p", (void *)base);
                return true;
            }
        }
        if (!base)
            base = resolve_il2cpp_base();
        if (base)
            LOGI("fallback game base %p", (void *)base);
        return base != 0;
    };
    if (!try_bases())
    {
        LOGI("update::init no game base — menu draw only");
        return;
    }

    // Wire touch/screen function pointers by RVA only — DO NOT call ::init()/
    // img_to_asm/il2cpp_domain_get (that was crashing on the GL thread).
    LOGI("update::init wire touch RVAs...");
    if (c_globals)
        c_globals->init();
    LOGI("update::init touch RVAs wired");

    g_sdk_ready.store(true, std::memory_order_release);
    LOGI("update::init menu-only done (no il2cpp domain / no VMT / no ray)");
}
