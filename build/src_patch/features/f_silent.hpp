#pragma once

#include "../core.hpp"
#include "../events.hpp"
#include "../sdk/include.h"
#include "f_aimbot.hpp"

namespace silent {

struct ray_t {
    Vector3 origin;
    Vector3 direction;
};

struct raycast_hit_t {
    Vector3 point;
    Vector3 normal;
    uint32_t face;
    float distance;
    Vector2 uv;
    int collider;
};

inline bool hooked = false;
inline bool (*orig_raycast)(void*, ray_t*, float, raycast_hit_t*, int32_t, uint8_t) = nullptr;

inline Vector3 target_pos{};
inline bool has_target = false;

static Vector3 cam_pos(uint64_t lp) {
    Vector3 out{};
    if (!ok(lp)) return out;
    uint64_t cam = rd64(lp + OFF_PLAYER_MAIN_CAMERA);
    if (ok(cam)) {
        uint64_t tr = rd64(cam + 0x20);
        if (!ok(tr)) tr = rd64(cam + OFF_CAMERA_TRANSFORM);
        Vector3 p{};
        if (ok(tr) && tp(tr, p)) return p;
        uint64_t holder = rd64(lp + OFF_PLAYER_CAMERA_HOLDER);
        if (ok(holder) && tp(holder, p)) return p;
    }
    out = player_pos(lp);
    out.y += 1.18f;
    return out;
}

static Vector3 norm3(Vector3 v) {
    float l = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (l < 1e-6f) return Vector3{0, 0, 1};
    return Vector3{v.x / l, v.y / l, v.z / l};
}

static bool pick_target(Vector3& out) {
    has_target = false;
    uint64_t pm = player_manager();
    if (!ok(pm)) return false;
    uint64_t lp = rd64(pm + OFF_PM_LOCAL_PLAYER);
    if (!ok(lp)) return false;
    Vector3 eye = cam_pos(lp);
    int lteam = rd8(lp + OFF_PLAYER_TEAM);
    float vm[16];
    bool have_vm = view_matrix(vm);
    float cx = (float)scr_w * 0.5f, cy = (float)scr_h * 0.5f;
    float rad = aimbot::fov_radius_px(s_aim_fov, (float)scr_h);
    float best = 1e30f;
    Vector3 best_pos{};
    bool found = false;
    int bone = aimbot::aim_bone_idx();

    uint64_t players[96];
    int np = collect_players(pm, lp, players, 96);
    for (int i = 0; i < np; i++) {
        uint64_t e = players[i];
        if (!ok(e)) continue;
        if (rd8(e + OFF_PLAYER_TEAM) == lteam) continue;
        if (health_of(e) <= 0) continue;
        if (opt_aim_visible && !player_visible(e)) continue;
        SK s;
        if (!gsb(e, s) || !s.ok) continue;

        int bones[4] = { bone, BONE_HEAD, BONE_NECK, BONE_SPINE2 };
        for (int bi = 0; bi < 4; bi++) {
            int b = bones[bi];
            if (bi > 0 && b == bone) continue;
            if (!s.v[b]) continue;
            Vector3 bp = s.b[b];
            if (b == BONE_HEAD) bp.y -= 0.25f;
            else if (b == BONE_NECK) bp.y -= 0.30f;
            if (!sane_world_pos(bp)) continue;

            float score;
            if (have_vm && rad > 1.f) {
                float sx, sy;
                if (!w2s(bp, vm, sx, sy)) continue;
                float dx = sx - cx, dy = sy - cy;
                float d = sqrtf(dx * dx + dy * dy);
                if (d > rad) continue;
                score = d;
            } else {
                float dx = bp.x - eye.x, dy = bp.y - eye.y, dz = bp.z - eye.z;
                score = sqrtf(dx * dx + dy * dy + dz * dz);
            }
            if (score < best) {
                best = score;
                best_pos = bp;
                found = true;
            }
            break;
        }
    }
    if (!found) return false;
    out = best_pos;
    target_pos = best_pos;
    has_target = true;
    return true;
}

static bool hk_raycast(void* scene, ray_t* ray, float max_distance, raycast_hit_t* hit, int32_t layer, uint8_t trigger) {
    if (opt_silent && ray && layer == 1610637328 && max_distance == 1000.0f) {
        Vector3 pos{};
        if (pick_target(pos)) {
            uint64_t pm = player_manager();
            uint64_t lp = ok(pm) ? rd64(pm + OFF_PM_LOCAL_PLAYER) : 0;
            Vector3 eye = cam_pos(lp);
            Vector3 dir{pos.x - eye.x, pos.y - eye.y, pos.z - eye.z};
            ray->direction = norm3(dir);
        }
    }
    if (!orig_raycast) return false;
    return orig_raycast(scene, ray, max_distance, hit, layer, trigger);
}

static void try_hook() {
    if (hooked || !il2cpp::resolve_icall) return;
    void* fn = il2cpp::resolve_icall(
        "UnityEngine.PhysicsScene::Internal_Raycast_Injected(UnityEngine.PhysicsScene&,UnityEngine.Ray&,System.Single,UnityEngine.RaycastHit&,System.Int32,UnityEngine.QueryTriggerInteraction)");
    if (!fn) {
        fn = il2cpp::resolve_icall("UnityEngine.Physics::Raycast");
    }
    if (!fn) return;
    inline_hook(fn, (void*)hk_raycast, (void**)&orig_raycast);
    if (orig_raycast) hooked = true;
}

static void cm(void* obj, void* cmd) {
    (void)obj;
    if (!cmd) return;
    if (!(opt_silent && opt_autofire)) return;
    Vector3 pos{};
    if (!pick_target(pos)) return;
    *(uint8_t*)((uintptr_t)cmd + 0x21) = 1;
}

static void late(void* p, bool local) {
    (void)p;
    if (!local) return;
    try_hook();
    if (opt_silent) {
        Vector3 pos{};
        pick_target(pos);
    } else {
        has_target = false;
    }
}

static void reset() {
    hooked = false;
    orig_raycast = nullptr;
    has_target = false;
}

}

namespace {
static events::feature _f_silent = {"silent", nullptr, silent::late, silent::cm, nullptr, silent::reset};
static struct _reg_silent { _reg_silent() { events::register_feature(_f_silent); } } _r_silent;
}
