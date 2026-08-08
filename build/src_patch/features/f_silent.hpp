#pragma once

#include "../core.hpp"
#include "../events.hpp"
#include "../sdk/include.h"

// Silent aim + autofire ported from internal-main (raycast redirect + GunController ExecuteCommands).
// Logic mirrors update.cpp / globals::updateTarget; adapted to this SDK + 0.39.2 offsets.

namespace silent {

#pragma pack(push, 1)
struct weapon_controller_cmd {
    bool to_fire;
    bool to_aim;
    bool to_reload;
    bool to_action;
    bool to_inspect;
    bool to_unknown;
};
#pragma pack(pop)

struct ray_t {
    Vector3 origin;     // m_vecOrigin
    Vector3 direction;  // m_vecDirection
};

struct raycast_hit_t {
    Vector3 point;
    Vector3 normal;
    uint32_t face;
    float distance;
    Vector2 uv;
    int collider;
};

inline bool ray_hooked = false;
inline bool exec_hooked = false;
inline bool (*orig_raycast)(void*, ray_t*, float, raycast_hit_t*, int32_t, uint8_t) = nullptr;
inline void (*orig_execute)(void*, weapon_controller_cmd, float, float) = nullptr;
inline const Il2CppMethod* exec_mi = nullptr;

inline Vector3 target_pos{};
inline bool has_target = false;
inline bool firing = false;

inline bool (*linecast_fn)(Vector3, Vector3, raycast_hit_t*, int32_t) = nullptr;

static Vector3 normalize_dir(Vector3 v) {
    float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (l < 1e-6f) return Vector3{0.f, 0.f, 1.f};
    return Vector3{v.x / l, v.y / l, v.z / l};
}

static Vector3 camera_pos(uint64_t lp) {
    Vector3 out{};
    if (!ok(lp)) return out;
    // internal-main: local->m_pMainCameraHolder->get_position()
    uint64_t holder = rd64(lp + OFF_PLAYER_CAMERA_HOLDER);
    if (ok(holder)) {
        Vector3 p{};
        if (tp(holder, p) && sane_world_pos(p)) return p;
        uint64_t native = rd64(holder + 0x10);
        if (ok(native) && tp(holder, p)) return p;
    }
    uint64_t cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (ok(cam)) {
        uint64_t tr = rd64(cam + 0x20);
        Vector3 p{};
        if (ok(tr) && tp(tr, p) && sane_world_pos(p)) return p;
    }
    out = player_pos(lp);
    out.y += 1.18f;
    return out;
}

static bool bone_visible(Vector3 start, Vector3 end) {
    if (!linecast_fn) {
        // fallback: occlusion flag on enemy is checked by caller when opt_aim_visible
        return true;
    }
    raycast_hit_t hit{};
    // internal-main layerMask 16384
    return !linecast_fn(start, end, &hit, 16384);
}

static bool hitbox_enabled() {
    return opt_hitbox[0] || opt_hitbox[1] || opt_hitbox[2] || opt_hitbox[3];
}

// mirrors globals::updateTarget / hook_raycast bone pick from internal-main
static bool update_target() {
    has_target = false;
    target_pos = Vector3{};
    if (!opt_silent || !hitbox_enabled()) return false;

    uint64_t pm = player_manager();
    if (!ok(pm)) return false;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp) || health_of(lp) <= 0) return false;

    Vector3 eye = camera_pos(lp);
    if (!sane_world_pos(eye)) return false;
    int lteam = rd8(lp + OFF_PLAYER_TEAM);

    float best_dist = 1e30f;
    Vector3 best{};
    bool found = false;

    uint64_t players[96];
    int np = collect_players(pm, lp, players, 96);
    for (int i = 0; i < np; i++) {
        uint64_t e = players[i];
        if (!ok(e)) continue;
        if (health_of(e) <= 0) continue;
        if (rd8(e + OFF_PLAYER_TEAM) == lteam) continue;
        if (opt_aim_visible && !player_visible(e)) continue;

        SK s;
        if (!gsb(e, s) || !s.ok) continue;

        // hitbox groups exactly like internal-main
        // [0]=head (head,neck) [1]=body (spine,spine1,toes) [2]=hip [3]=legs
        int head_b[] = { BONE_HEAD, BONE_NECK };
        int body_b[] = { BONE_SPINE, BONE_SPINE1, BONE_LEFT_TOE, BONE_RIGHT_TOE };
        int arms_b[] = { BONE_HIP };
        int legs_b[] = { BONE_LEFT_LEG, BONE_LEFT_UPLEG, BONE_RIGHT_LEG, BONE_RIGHT_UPLEG };

        struct group { bool en; int* bones; int n; };
        group groups[4] = {
            { opt_hitbox[1], body_b, 4 },
            { opt_hitbox[0], head_b, 2 },
            { opt_hitbox[2], arms_b, 1 },
            { opt_hitbox[3], legs_b, 4 },
        };

        for (int w = 0; w < 4; w++) {
            if (!groups[w].en) continue;
            for (int j = 0; j < groups[w].n; j++) {
                int bi = groups[w].bones[j];
                if (!s.v[bi]) continue;
                Vector3 bonepos = s.b[bi];
                if (!sane_world_pos(bonepos)) continue;
                if (!bone_visible(eye, bonepos)) continue;
                float dx = bonepos.x - eye.x, dy = bonepos.y - eye.y, dz = bonepos.z - eye.z;
                float dist = sqrtf(dx * dx + dy * dy + dz * dz);
                if (dist < best_dist) {
                    best_dist = dist;
                    best = bonepos;
                    found = true;
                }
            }
        }
    }

    if (!found) return false;
    target_pos = best;
    has_target = true;
    return true;
}

// internal-main hook_raycast
static bool hk_raycast(void* scene, ray_t* ray, float max_distance, raycast_hit_t* hit, int32_t layer, uint8_t trigger) {
    if (opt_silent && hitbox_enabled() && ray && layer == 1610637328 && max_distance == 1000.0f) {
        uint64_t pm = player_manager();
        uint64_t lp = ok(pm) ? rd64(pm + OFF_PM_LOCAL_PLAYER) : 0;
        if (ok(lp)) {
            Vector3 eye = camera_pos(lp);
            Vector3 pos{};
            bool b_found = false;
            float best_dist = 1e30f;
            int lteam = rd8(lp + OFF_PLAYER_TEAM);

            uint64_t players[96];
            int np = collect_players(pm, lp, players, 96);
            for (int i = 0; i < np; i++) {
                uint64_t e = players[i];
                if (!ok(e) || health_of(e) <= 0) continue;
                if (rd8(e + OFF_PLAYER_TEAM) == lteam) continue;
                SK s;
                if (!gsb(e, s) || !s.ok) continue;

                int head_b[] = { BONE_HEAD, BONE_NECK };
                int body_b[] = { BONE_SPINE, BONE_SPINE1, BONE_LEFT_TOE, BONE_RIGHT_TOE };
                int arms_b[] = { BONE_HIP };
                int legs_b[] = { BONE_LEFT_LEG, BONE_LEFT_UPLEG, BONE_RIGHT_LEG, BONE_RIGHT_UPLEG };
                struct group { bool en; int* bones; int n; };
                group groups[4] = {
                    { opt_hitbox[1], body_b, 4 },
                    { opt_hitbox[0], head_b, 2 },
                    { opt_hitbox[2], arms_b, 1 },
                    { opt_hitbox[3], legs_b, 4 },
                };
                for (int w = 0; w < 4; w++) {
                    if (!groups[w].en) continue;
                    for (int j = 0; j < groups[w].n; j++) {
                        int bi = groups[w].bones[j];
                        if (!s.v[bi]) continue;
                        Vector3 bonepos = s.b[bi];
                        if (!sane_world_pos(bonepos)) continue;
                        if (!bone_visible(eye, bonepos)) continue;
                        float dx = bonepos.x - eye.x, dy = bonepos.y - eye.y, dz = bonepos.z - eye.z;
                        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
                        if (dist < best_dist) {
                            best_dist = dist;
                            pos = bonepos;
                            b_found = true;
                        }
                    }
                }
            }
            if (b_found) {
                Vector3 dir{pos.x - eye.x, pos.y - eye.y, pos.z - eye.z};
                ray->direction = normalize_dir(dir);
                target_pos = pos;
                has_target = true;
            }
        }
    }
    if (!orig_raycast) return false;
    return orig_raycast(scene, ray, max_distance, hit, layer, trigger);
}

// internal-main hook_executecommands
static void hk_execute(void* thiz, weapon_controller_cmd commands, float duration, float time) {
    if (!thiz) {
        if (orig_execute) orig_execute(thiz, commands, duration, time);
        return;
    }
    firing = commands.to_fire;

    if (opt_autofire && opt_silent) {
        uint64_t pm = player_manager();
        uint64_t lp = ok(pm) ? rd64(pm + OFF_PM_LOCAL_PLAYER) : 0;
        if (ok(lp) && health_of(lp) > 0) {
            update_target();
            if (has_target) {
                commands.to_fire = true;
                firing = true;
            }
        }
    }

    if (orig_execute) orig_execute(thiz, commands, duration, time);
}

static const Il2CppMethod* find_execute_method(Il2CppClass* gun) {
    if (!gun || !il2cpp::class_get_methods || !il2cpp::method_get_param_count || !il2cpp::method_get_param || !il2cpp::type_get_type)
        return nullptr;
    void* it = nullptr;
    const Il2CppMethod* found = nullptr;
    while (const Il2CppMethod* m = il2cpp::class_get_methods(gun, &it)) {
        if (il2cpp::method_get_param_count(m) != 3) continue;
        const Il2CppType* p1 = il2cpp::method_get_param(m, 1);
        const Il2CppType* p2 = il2cpp::method_get_param(m, 2);
        if (!p1 || !p2) continue;
        // float = IL2CPP_TYPE_R4 (0x0C)
        if (il2cpp::type_get_type(p1) == 0x0C && il2cpp::type_get_type(p2) == 0x0C) {
            found = m;
            // prefer non-generic instance methods; keep last matching (ExecuteCommands is typically late)
        }
    }
    // also try known obfuscated name from older internal-main builds
    if (!found && il2cpp::class_get_method_from_name) {
        found = il2cpp::class_get_method_from_name(gun, "FEEBGAGHGGCGACA", 3);
    }
    if (!found) found = find_method(gun, "FEEBGAGHGGCGACA");
    return found;
}

static void try_hook_ray() {
    if (ray_hooked || !il2cpp::resolve_icall) return;
    void* fn = il2cpp::resolve_icall(
        "UnityEngine.PhysicsScene::Internal_Raycast_Injected(UnityEngine.PhysicsScene&,UnityEngine.Ray&,System.Single,UnityEngine.RaycastHit&,System.Int32,UnityEngine.QueryTriggerInteraction)");
    if (!fn) return;
    inline_hook(fn, (void*)hk_raycast, (void**)&orig_raycast);
    if (orig_raycast) ray_hooked = true;

    if (!linecast_fn) {
        void* lc = il2cpp::resolve_icall(
            "UnityEngine.Physics::Linecast(UnityEngine.Vector3,UnityEngine.Vector3,UnityEngine.RaycastHit&,System.Int32)");
        if (!lc)
            lc = il2cpp::resolve_icall(
                "UnityEngine.Physics::Linecast_Injected(UnityEngine.Vector3&,UnityEngine.Vector3&,UnityEngine.RaycastHit&,System.Int32)");
        linecast_fn = (decltype(linecast_fn))lc;
    }
}

static void try_hook_execute() {
    if (exec_hooked) return;
    Il2CppClass* gun = sdk::class_lazy("Assembly-CSharp", "Axlebolt.Standoff.Inventory.Gun", "GunController");
    if (!gun) return;
    const Il2CppMethod* m = find_execute_method(gun);
    if (!m) return;
    exec_mi = m;
    uintptr_t* slot = (uintptr_t*)((uintptr_t)m + il2cpp::offset::il2cpp_method_pointer);
    if (!ok(*slot)) return;
    if (!mi_make_rw((uintptr_t)slot)) return;
    if (!orig_execute) orig_execute = (decltype(orig_execute))*slot;
    *slot = (uintptr_t)hk_execute;
    exec_hooked = true;
}

static void late(void* p, bool local) {
    (void)p;
    if (!local) return;
    try_hook_ray();
    try_hook_execute();
    if (opt_silent) update_target();
    else { has_target = false; firing = false; }
}

// CreateMove fallback (same fire bit path if ExecuteCommands hook not resolved yet)
static void cm(void* obj, void* cmd) {
    (void)obj;
    if (!cmd) return;
    if (!(opt_autofire && opt_silent)) return;
    if (!has_target) update_target();
    if (!has_target) return;
    *(uint8_t*)((uintptr_t)cmd + 0x21) = 1;
    firing = true;
}

static void reset() {
    ray_hooked = false;
    exec_hooked = false;
    orig_raycast = nullptr;
    orig_execute = nullptr;
    exec_mi = nullptr;
    has_target = false;
    firing = false;
    linecast_fn = nullptr;
}

}

namespace {
static events::feature _f_silent = {"silent", nullptr, silent::late, silent::cm, nullptr, silent::reset};
static struct _reg_silent { _reg_silent() { events::register_feature(_f_silent); } } _r_silent;
}
