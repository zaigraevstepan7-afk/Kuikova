#include "globals.hpp"
#include "includes/halalium_mem.h"
#include "includes/halalium_chains.h"
#include "includes/module_base.h"
#include "sdk/game/il2cpp/api.h"
#include <unistd.h>
#include <thread>
#include <cmath>
#include <cstring>
#include "src/features/exploits.h"

bool globals::is_allocated(void *x)
{
    // Halalium-style: maps check instead of mincore / memfd / process_vm_*.
    return hmem::readable(reinterpret_cast<uintptr_t>(x), sizeof(void *));
}

void *globals::get_lazysingleton_typeinfo(uintptr_t addr)
{
    const uintptr_t base_addr = reinterpret_cast<uintptr_t>(base);
    if (!base_addr || !addr)
        return nullptr;

    // Halalium: ldr [base + TypeInfoRVA]; follow chain with null checks.
    void *offset = reinterpret_cast<void *>(hmem::read_ptr(base_addr + addr));
    if (!offset)
        return nullptr;
    void *offset2 = hmem::field_ptr(offset, oxorany(0x58));
    if (!offset2)
        return nullptr;
    // 0.39.2: Il2CppClass.static_fields @ 0x90 (was 0xB8 on older layouts)
    void *offset3 = hmem::field_ptr(offset2, oxorany(0x90));
    if (!offset3)
        return nullptr;
    return hmem::field_ptr(offset3, oxorany(0x0));
}

void *globals::type_info_instance(uintptr_t addr, uintptr_t field)
{
    const uintptr_t base_addr = reinterpret_cast<uintptr_t>(base);
    if (!base_addr || !addr)
        return nullptr;

    // Halalium: klass = *(base + TypeInfoRVA); instance from static_fields.
    void *type_info = hmem::typeinfo(base_addr, addr);
    if (!type_info)
        return nullptr;

    void *static_field = hmem::field_ptr(type_info, c_offsets->il2cpp_static_fields);
    if (!static_field)
        return nullptr;

    // Try requested slot, then common alternates (0x10 / 0x0)
    const uintptr_t slots[3] = {field, static_cast<uintptr_t>(0x10), static_cast<uintptr_t>(0x0)};
    for (int i = 0; i < 3; i++)
    {
        void *instance = hmem::field_ptr(static_field, slots[i]);
        if (instance)
            return instance;
    }
    return nullptr;
}

Vector3 globals::world2screen(const Matrix &viewMatrix, const Vector3 &pos)
{
    float clipX = pos.x * viewMatrix.m00 + pos.y * viewMatrix.m01 + pos.z * viewMatrix.m02 + viewMatrix.m03;
    float clipY = pos.x * viewMatrix.m10 + pos.y * viewMatrix.m11 + pos.z * viewMatrix.m12 + viewMatrix.m13;
    float clipZ = pos.x * viewMatrix.m20 + pos.y * viewMatrix.m21 + pos.z * viewMatrix.m22 + viewMatrix.m23;
    float clipW = pos.x * viewMatrix.m30 + pos.y * viewMatrix.m31 + pos.z * viewMatrix.m32 + viewMatrix.m33;

    if (clipW < 0.001f)
        return {0.f, 0.f, -1.f};

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;
    float ndcZ = clipZ / clipW;

    return {
        (ndcX * 0.5f + 0.5f) * c_egl->width,
        (1.f - (ndcY * 0.5f + 0.5f)) * c_egl->heigth,
        ndcZ
    };
}

Vector3 world2screen(const Matrix &vm, const Vector3 &vecOrigin) {
    const Quaternion qClip {
        vecOrigin.x * vm.m00 + vecOrigin.y * vm.m01 + vecOrigin.z * vm.m02 + vm.m03,
        vecOrigin.x * vm.m10 + vecOrigin.y * vm.m11 + vecOrigin.z * vm.m12 + vm.m13,
        vecOrigin.x * vm.m20 + vecOrigin.y * vm.m21 + vecOrigin.z * vm.m22 + vm.m23,
        vecOrigin.x * vm.m30 + vecOrigin.y * vm.m31 + vecOrigin.z * vm.m32 + vm.m33,
    };

    const Vector3 vecNormalized {
        qClip.x / (qClip.w > 0 ? qClip.w : 1),
        qClip.y / (qClip.w > 0 ? qClip.w : 1),
        qClip.z / (qClip.w > 0 ? qClip.w : 1)
    };

    return Vector3 {
        (c_egl->width / 2 * vecNormalized.x) + (vecNormalized.x + c_egl->width / 2),
        -(c_egl->heigth / 2 * vecNormalized.y) + (vecNormalized.y + c_egl->heigth / 2),
        vecNormalized.z
    };
}

bool globals::is_bone_visible(Vector3 start, Vector3 end)
{
    raycast_hit_t ray{};
    return !c_methods->linecast(start, end, &ray, 16384);
}

bool globals::is_alive(c_player_controller *player)
{
    if (!player)
        return false;

    auto photon = player->m_pPhoton;
    if (!photon)
        return false;

    int32_t health = photon->get_health();
    return health > 0;
}

bool globals::is_enemy(c_player_controller *local, c_player_controller *enemy)
{
    if (!local || !enemy)
        return false;

    team_t local_team = local->m_team;
    team_t enemy_team = enemy->m_team;

    return local_team != enemy_team;
}

bool globals::is_sniper()
{
    c_weapon_parameters *params{};
    uint8_t m_id{};

    params = c_player->weapon_parameters;
    if (!params)
        return false;

    m_id = params->m_id;

    return m_id >= 51 && m_id <= 54;
}

bool globals::holding_gun()
{
    c_weapon_parameters *params{};
    uint8_t m_id{};

    params = c_player->weapon_parameters;
    if (!params)
        return false;

    m_id = params->m_id;

    return m_id >= 11 && m_id <= 69;
}

bool globals::is_scoped() {
    if (!c_player->local)
        return false;

    auto gun_controller = c_player->gun_controller;
    auto gun_parameters = c_player->gun_parameters;

    if (!gun_controller) return false;
    if (!gun_parameters) return false;

    if (!c_globals->holding_gun())
        return false;

    auto sight_type = gun_parameters->_sightType;
    if (sight_type == _sightType::Default)
        return false;

    auto gun_state = gun_controller->m_pState;
    if (!gun_state)
        return false;

    auto aiming_mode = gun_controller->m_pAimMode;
    if (!aiming_mode)
        return false;

    auto aim = aiming_mode->m_curState;
    auto gun = gun_state->m_curState;

    return ((aim == 0 || aim == 3) && (gun != 0 && gun != 1));
}

void player::collect(c_player_controller* player)
{
    if (!player) return;
    if (!player->m_pPhoton) return;

    for (int i = 0; i < list.size(); i++) {
        if (list[i] == player) return;
    }

    list.push_back(player);
}

void player::reset()
{
    local = nullptr;

    for (int i = 0; i < list.size(); i++) {
        if (!list[i] || !list[i]->m_pPhoton) {
            list.erase(list.begin() + i);
            i--;
        }
    }
}

void player::after_match()
{
    local = nullptr;
    entity.clear();
    list.clear();
}

void player::update()
{
    local = nullptr;
    entity.clear();

    for (int i = 0; i < list.size(); i++) {
        auto* player = list[i];
        if (!player || !player->m_pPhoton) continue;

        auto* photon = player->m_pPhoton;
        if (photon->m_bIsLocal) {
            local = player;
        }

        entity.push_back(player);
    }
}

void globals::updateGun()
{
    if (!c_player->local)
        return;

    // Halalium chain: player+0x88 → +0xa0 → params@+0x168 / +0xa8
    void *weaponry = hchain::weaponry(c_player->local);
    if (!weaponry)
        return;

    void *weapon = hchain::current_weapon(c_player->local);
    if (!weapon)
        return;

    c_player->weapon_controller = reinterpret_cast<c_weapon_controller *>(weapon);
    c_player->gun_controller = reinterpret_cast<c_gun_controller *>(weapon);

    void *gparams = hchain::gun_params(weapon);
    void *wparams = hchain::weapon_params_alt(weapon);
    c_player->gun_parameters = reinterpret_cast<c_gun_parameters *>(gparams);
    c_player->weapon_parameters = reinterpret_cast<c_weapon_parameters *>(wparams ? wparams : gparams);
    if (!c_player->weapon_parameters || !c_player->gun_parameters)
        return;

    if (!this->holding_gun())
        return;

    // Apply Halalium Safe writes (Inf Ammo / Fire Rate / Wallshot / No spread)
    c_exploits->init(c_player->gun_controller);
}

bool globals::in_fov(c_transform *cam, const Vector3 &cam_pos, const Vector3 &world, float fov_deg)
{
    if (!cam || fov_deg <= 0.f)
        return true;
    Vector3 fwd = cam->get_forward();
    Vector3 dir = (world - cam_pos).nnormalized();
    if (fwd == Vector3{} || dir == Vector3{})
        return true;
    const float flen = fwd.length();
    if (flen < 1e-4f)
        return true;
    fwd = fwd * (1.f / flen);
    float d = fwd.x * dir.x + fwd.y * dir.y + fwd.z * dir.z;
    if (d > 1.f)
        d = 1.f;
    if (d < -1.f)
        d = -1.f;
    const float ang = acosf(d) * (180.f / static_cast<float>(M_PI));
    return ang <= (fov_deg * 0.5f);
}

void globals::updateTarget() {
    // Halalium Silent Aim does NOT require Enable Esp
    if (!g.b_silent)
        return;

    target_t.pos = Vector3{};
    target_t.b_found = false;
    float best_dist = FLT_MAX;

    c_player_controller *local = c_player->local;
    if (!local)
        return;
    c_transform *main_camera = local->m_pMainCameraHolder;
    if (!main_camera)
        return;
    // Melodium: holder must have cached native transform @+0x10
    if (!*(void **)((uintptr_t)main_camera + 0x10))
        return;

    Vector3 camera_pos = main_camera->get_position();
    if (camera_pos == Vector3{})
        return;

    for (int i{}; i < (int)c_player->entity.size(); i++)
    {
        c_player_controller *player = c_player->entity[i];
        if (!player || !c_globals->is_alive(player) || !c_globals->is_enemy(local, player))
            continue;

        auto *c_character = player->m_pCharacterView;
        if (!c_character)
            continue;
        auto *c_biped = c_character->c_biped;
        if (!c_biped)
            continue;

        c_transform *_head[] = {c_biped->head, c_biped->neck};
        c_transform *_body[] = {c_biped->spine, c_biped->spine1, c_biped->left_toe_base, c_biped->right_toe_base};
        c_transform *_arms[] = {c_biped->hip};
        c_transform *_legs[] = {c_biped->left_leg, c_biped->left_up_leg, c_biped->right_leg, c_biped->right_up_leg};

        const bool bone_aim = g.b_silent_bone;
        struct hitbox_group
        {
            bool enabled;
            c_transform **bones;
            int count;
        };
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
                c_transform *bone = groups[w].bones[j];
                if (!bone)
                    continue;
                Vector3 bonepos = bone->get_position();
                if (g.b_fov_check && !in_fov(main_camera, camera_pos, bonepos, g.f_fov_check))
                    continue;
                if (!g.b_autowall && !c_globals->is_bone_visible(camera_pos, bonepos))
                    continue;
                float dist = (bonepos - camera_pos).length();
                if (dist < best_dist)
                {
                    best_dist = dist;
                    target_t.pos = bonepos;
                    target_t.b_found = true;
                }
            }
        }
    }
}

void globals::init()
{
    auto bind_u = [](uintptr_t rva) -> void * {
        if (!unity_base || !rva)
            return nullptr;
        void *p = (void *)(unity_base + rva);
        return maps_contains_exec((uintptr_t)p) ? p : nullptr;
    };

    if (unity_base)
    {
        c_fn->set_active = (decltype(c_fn->set_active))(unity_base + Offsets::UnityMethod::set_active);
        c_fn->set_fov = (decltype(c_fn->set_fov))(unity_base + Offsets::UnityMethod::set_fov);
        c_fn->get_game_object = (decltype(c_fn->get_game_object))(unity_base + Offsets::UnityMethod::get_game_object);
        c_methods->set_active = c_fn->set_active;

        // Melodium 0.39.2 Transform/Camera/Physics/TPS — proven working on device
        c_fn->get_position = (decltype(c_fn->get_position))bind_u(0x6005138);
        c_fn->set_position = (decltype(c_fn->set_position))bind_u(0x6009694);
        c_fn->get_forward = (decltype(c_fn->get_forward))bind_u(0x60062E0);
        c_fn->get_up = (decltype(c_fn->get_up))bind_u(0x6002608);
        c_fn->get_euler_angles = (decltype(c_fn->get_euler_angles))bind_u(0x6006BF0);
        c_fn->set_euler_angles = (decltype(c_fn->set_euler_angles))bind_u(0x5FEC6CC);
        c_fn->get_rotation = (decltype(c_fn->get_rotation))bind_u(0x5FF2184);
        c_fn->get_transform = (decltype(c_fn->get_transform))bind_u(0x5FF113C);
        c_fn->camera_get_main = (decltype(c_fn->camera_get_main))bind_u(0x5FACE04);
        c_fn->get_w2c_injected = (decltype(c_fn->get_w2c_injected))bind_u(0x5FBA5F4);
        c_fn->shader_find = (decltype(c_fn->shader_find))bind_u(0x6A95144);
        c_fn->mat_get_texture = (decltype(c_fn->mat_get_texture))bind_u(0x6A92FA4);
        c_fn->mat_set_texture = (decltype(c_fn->mat_set_texture))bind_u(0x6A81174);
        c_fn->mat_get_shader = (decltype(c_fn->mat_get_shader))bind_u(0x6A82020);
        c_fn->mat_set_shader = (decltype(c_fn->mat_set_shader))bind_u(0x6A88CA8);
        c_fn->mat_ctor_shader = (decltype(c_fn->mat_ctor_shader))bind_u(0x6A98518);
        c_fn->mat_set_color = (decltype(c_fn->mat_set_color))bind_u(0x6A96904);
        c_fn->mat_set_int = (decltype(c_fn->mat_set_int))bind_u(0x6A84BE4);
        c_fn->mat_set_float = (decltype(c_fn->mat_set_float))bind_u(0x6A8BF28);
        c_fn->renderer_get_material = (decltype(c_fn->renderer_get_material))bind_u(0x6A962B4);
        c_fn->renderer_set_material = (decltype(c_fn->renderer_set_material))bind_u(0x6A91498);
        c_fn->renderer_get_materials = (decltype(c_fn->renderer_get_materials))bind_u(0x6A9149C);
        c_fn->renderer_set_materials = (decltype(c_fn->renderer_set_materials))bind_u(0x6A8EFD8);
        c_fn->find_objects_of_type = (decltype(c_fn->find_objects_of_type))bind_u(0x5FF2770);
        c_fn->set_tps = (decltype(c_fn->set_tps))bind_u(0x8E7E63C);
        c_fn->set_fps = (decltype(c_fn->set_fps))bind_u(0x8E7EC48);
        c_fn->set_visible = (decltype(c_fn->set_visible))bind_u(0x8E880E4);
        c_fn->get_velocity = (decltype(c_fn->get_velocity))bind_u(0x7A05344);

        c_methods->linecast = (decltype(c_methods->linecast))bind_u(0x7A020F4);
        c_methods->sphere_cast = (decltype(c_methods->sphere_cast))bind_u(0x79FFF6C);
        c_methods->get_count = (decltype(c_methods->get_count))bind_u(0x684E370);
        c_methods->get_touch = (decltype(c_methods->get_touch))bind_u(0x684EDAC);
    }

    // MethodInfo fill-ins for anything Melodium RVA missed
    auto mp = [](Il2CppClass *clz, const char *name, uint8_t argc) -> void * {
        MethodInfo *m = GetMethodFromClass(clz, name, argc);
        return m ? (void *)m->methodPointer : nullptr;
    };
    auto mp_any = [](Il2CppClass *clz, const char *name) -> void * {
        if (!clz || !clz->methods)
            return nullptr;
        for (unsigned short i = 0; i < clz->method_count; i++)
        {
            auto method = ((MethodInfo **)clz->methods)[i];
            if (method && method->name && strcmp(method->name, name) == 0 && method->methodPointer)
                return (void *)method->methodPointer;
        }
        return nullptr;
    };

    auto fill = [&](auto &dst, auto src) { if (!dst && src) dst = src; };

    auto *tr = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Transform"));
    if (tr)
    {
        fill(c_fn->get_position, (decltype(c_fn->get_position))mp(tr, oxorany("get_position"), 0));
        fill(c_fn->set_position, (decltype(c_fn->set_position))mp(tr, oxorany("set_position"), 1));
        fill(c_fn->get_forward, (decltype(c_fn->get_forward))mp(tr, oxorany("get_forward"), 0));
        fill(c_fn->get_up, (decltype(c_fn->get_up))mp(tr, oxorany("get_up"), 0));
        fill(c_fn->get_euler_angles, (decltype(c_fn->get_euler_angles))mp(tr, oxorany("get_eulerAngles"), 0));
        fill(c_fn->set_euler_angles, (decltype(c_fn->set_euler_angles))mp(tr, oxorany("set_eulerAngles"), 1));
        fill(c_fn->get_rotation, (decltype(c_fn->get_rotation))mp(tr, oxorany("get_rotation"), 0));
    }

    auto *comp = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Component"));
    if (comp)
        fill(c_fn->get_transform, (decltype(c_fn->get_transform))mp(comp, oxorany("get_transform"), 0));

    auto *go = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("GameObject"));
    if (go)
        fill(c_fn->set_active, (decltype(c_fn->set_active))mp(go, oxorany("SetActive"), 1));

    auto *cam = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Camera"));
    if (cam)
    {
        fill(c_fn->camera_get_main, (decltype(c_fn->camera_get_main))mp(cam, oxorany("get_main"), 0));
        fill(c_fn->set_fov, (decltype(c_fn->set_fov))mp(cam, oxorany("set_fieldOfView"), 1));
        fill(c_fn->get_w2c_injected, (decltype(c_fn->get_w2c_injected))mp_any(cam, oxorany("get_worldToCameraMatrix_Injected")));
    }

    auto *inp = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Input"));
    if (inp)
    {
        fill(c_methods->get_count, (decltype(c_methods->get_count))mp(inp, oxorany("get_touchCount"), 0));
        fill(c_methods->get_touch, (decltype(c_methods->get_touch))mp(inp, oxorany("GetTouch"), 1));
    }

    auto *phys = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Physics"));
    if (phys)
    {
        fill(c_methods->linecast, (decltype(c_methods->linecast))mp_any(phys, oxorany("Linecast")));
        fill(c_methods->sphere_cast, (decltype(c_methods->sphere_cast))mp_any(phys, oxorany("SphereCast")));
    }

    auto *str = (Il2CppClass *)il2cpp_class_from_name(dll::charp ? dll::charp : dll::unity,
                                                     oxorany("System"), oxorany("String"));
    if (!str && dll::unity)
        str = (Il2CppClass *)clazz_unity(oxorany("System"), oxorany("String"));
    if (str)
        c_methods->create_string = (decltype(c_methods->create_string))mp_any(str, oxorany("CreateString"));

    auto *sh = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Shader"));
    if (sh)
        fill(c_fn->shader_find, (decltype(c_fn->shader_find))mp(sh, oxorany("Find"), 1));

    auto *mat = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Material"));
    if (mat)
    {
        fill(c_fn->mat_get_texture, (decltype(c_fn->mat_get_texture))mp(mat, oxorany("get_mainTexture"), 0));
        fill(c_fn->mat_set_texture, (decltype(c_fn->mat_set_texture))mp(mat, oxorany("set_mainTexture"), 1));
        fill(c_fn->mat_get_shader, (decltype(c_fn->mat_get_shader))mp(mat, oxorany("get_shader"), 0));
        fill(c_fn->mat_set_shader, (decltype(c_fn->mat_set_shader))mp(mat, oxorany("set_shader"), 1));
        fill(c_fn->mat_ctor_shader, (decltype(c_fn->mat_ctor_shader))mp(mat, oxorany(".ctor"), 1));
        fill(c_fn->mat_set_color, (decltype(c_fn->mat_set_color))mp_any(mat, oxorany("set_color")));
        fill(c_fn->mat_set_int, (decltype(c_fn->mat_set_int))mp_any(mat, oxorany("SetInt")));
        fill(c_fn->mat_set_float, (decltype(c_fn->mat_set_float))mp_any(mat, oxorany("SetFloat")));
    }

    auto *rend = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Renderer"));
    if (rend)
    {
        fill(c_fn->renderer_get_material, (decltype(c_fn->renderer_get_material))mp(rend, oxorany("get_material"), 0));
        fill(c_fn->renderer_set_material, (decltype(c_fn->renderer_set_material))mp(rend, oxorany("set_material"), 1));
        fill(c_fn->renderer_get_materials, (decltype(c_fn->renderer_get_materials))mp(rend, oxorany("get_materials"), 0));
        fill(c_fn->renderer_set_materials, (decltype(c_fn->renderer_set_materials))mp_any(rend, oxorany("SetMaterialArray")));
        fill(c_fn->renderer_set_materials, (decltype(c_fn->renderer_set_materials))mp(rend, oxorany("set_materials"), 1));
    }

    auto *obj = (Il2CppClass *)clazz_unity(oxorany("UnityEngine"), oxorany("Object"));
    if (obj)
        fill(c_fn->find_objects_of_type, (decltype(c_fn->find_objects_of_type))mp_any(obj, oxorany("FindObjectsOfType")));

    auto *pc = (Il2CppClass *)clazz_def(oxorany("Axlebolt.Standoff.Player"), oxorany("PlayerController"));
    if (pc)
    {
        fill(c_fn->set_tps, (decltype(c_fn->set_tps))mp_any(pc, oxorany("SetThirdPerson")));
        fill(c_fn->set_fps, (decltype(c_fn->set_fps))mp_any(pc, oxorany("SetFirstPerson")));
        fill(c_fn->set_visible, (decltype(c_fn->set_visible))mp_any(pc, oxorany("RefreshVisibility")));
    }

    LOGI("resolve: pos=%p fov=%p touch=%p linecast=%p shader=%p tps=%p",
         (void *)c_fn->get_position, (void *)c_fn->set_fov,
         (void *)c_methods->get_touch, (void *)c_methods->linecast,
         (void *)c_fn->shader_find, (void *)c_fn->set_tps);
}
