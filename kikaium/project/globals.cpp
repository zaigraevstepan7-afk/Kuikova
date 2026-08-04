#include "globals.hpp"
#include "includes/halalium_mem.h"
#include <unistd.h>
#include <thread>
#include <cmath>
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
    c_weaponry_controller *weaponry{};

    weaponry = c_player->local->m_pWeaponry;
    if (!weaponry)
        return;

    c_player->weapon_controller = weaponry->m_pCurrentWeapon;
    c_player->gun_controller = (c_gun_controller *)weaponry->m_pCurrentWeapon;
    if (!c_player->weapon_controller || !c_player->gun_controller)
        return;

    c_player->weapon_parameters = c_player->weapon_controller->m_pParameters;
    c_player->gun_parameters = c_player->gun_controller->m_pParameters;
    if (!c_player->weapon_parameters || !c_player->gun_parameters)
        return;

    if (!this->holding_gun()) return;

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
    c_methods->get_width = (decltype(c_methods->get_width))(base + c_offsets->get_width);
    c_methods->get_heigth = (decltype(c_methods->get_heigth))(base + c_offsets->get_heigth);
    c_methods->get_touch = (decltype(c_methods->get_touch))(base + c_offsets->get_touch);
    c_methods->get_count = (decltype(c_methods->get_count))(base + c_offsets->get_count);
    c_methods->linecast = (decltype(c_methods->linecast))(base + c_offsets->linecast);
    c_methods->sphere_cast = (decltype(c_methods->sphere_cast))(base + c_offsets->sphere_cast);
    c_methods->create_string = (decltype(c_methods->create_string))(base + c_offsets->create_string);
    c_methods->set_active = (decltype(c_methods->set_active))(base + c_offsets->set_active);
    c_methods->set_flags = (decltype(c_methods->set_flags))(base + c_offsets->set_flags);
    c_methods->object_instantiate = (decltype(c_methods->object_instantiate))(base + c_offsets->object_instantiate);
    c_methods->object_destroy = (decltype(c_methods->object_destroy))(base + c_offsets->object_destroy);
    c_methods->is_native_object_alive = (decltype(c_methods->is_native_object_alive))(base + c_offsets->is_native_object_alive);
}