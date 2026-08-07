#include "game.hpp"
#include "offsets.hpp"
#include "mem.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/uio.h>
#include <android/log.h>

#define NOVA_LOG(...) __android_log_print(ANDROID_LOG_INFO, "nova", __VA_ARGS__)

namespace {

uintptr_t module_base(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        // Prefer executable mapping
        if (line.find("r-xp") == std::string::npos && line.find("r--p") == std::string::npos) continue;
        uintptr_t start = 0;
        if (sscanf(line.c_str(), "%lx-", &start) == 1 && start) return start;
    }
    return 0;
}

uintptr_t resolve_manager(uintptr_t il2cpp) {
    const uintptr_t ti = il2cpp + off::kPlayerManagerTI;
    // TypeInfo -> static_fields (0x90) -> +0x10 -> +0x0
    const uintptr_t sf = mem::read_ptr(ti + off::mgr::kStaticFields);
    if (!sf) return 0;
    const uintptr_t p2 = mem::read_ptr(sf + off::mgr::kPtr2);
    if (!p2) return 0;
    return mem::read_ptr(p2 + off::mgr::kPtr3);
}

bool read_matrix(uintptr_t local, Mat4& out) {
    if (!local) return false;
    // local -> main_camera (0xE8)
    uintptr_t cam = mem::read_ptr(local + off::player::kMainCamera);
    if (!cam) return false;
    // camera nest: +0x20 -> +0x10 -> matrix @ +0xF0  (AcademicDLC camera)
    uintptr_t t = mem::read_ptr(cam + off::cam::kTransform);
    if (!t) {
        // alternate: holder at player+0x28 then camera component
        const uintptr_t holder = mem::read_ptr(local + off::player::kMainCameraHolder);
        if (holder) {
            cam = holder;
            t = mem::read_ptr(cam + off::cam::kTransform);
        }
    }
    if (!t) return false;
    const uintptr_t p = mem::read_ptr(t + off::cam::kPtr);
    if (!p) return false;
    return mem::read_into(p + off::cam::kMatrix, out);
}

bool read_transform_pos(uintptr_t transform, Vec3& out) {
    if (!transform) return false;
    const uintptr_t data = mem::read_ptr(transform + off::xform::kData);
    if (!data) return false;
    return mem::read_into(data + off::xform::kPosition, out);
}

bool read_bone_pos(uintptr_t player, int bone_off, Vec3& out) {
    const uintptr_t view = mem::read_ptr(player + off::player::kCharacterView);
    if (!view) return false;
    const uintptr_t map = mem::read_ptr(view + off::biped::kBipedMap);
    if (!map) return false;
    const uintptr_t bone = mem::read_ptr(map + bone_off);
    if (!bone) return false;
    const uintptr_t xf = mem::read_ptr(bone + off::biped::kTransformObject);
    return read_transform_pos(xf, out);
}

bool read_feet_fallback(uintptr_t player, Vec3& out) {
    // movement_controller -> translation data path (Academic movement.translation_data = 0xB0)
    const uintptr_t mov = mem::read_ptr(player + off::player::kMovement);
    if (!mov) return false;
    const uintptr_t data = mem::read_ptr(mov + 0xB0);
    if (!data) return false;
    return mem::read_into(data + off::xform::kPosition, out);
}

std::string read_il2cpp_string(uintptr_t str) {
    if (!str) return {};
    // Il2CppString: length @ +0x10, chars @ +0x14 (utf16)
    const int32_t len = mem::read<int32_t>(str + 0x10, 0);
    if (len <= 0 || len > 64) return {};
    std::vector<char16_t> u16(static_cast<size_t>(len));
    iovec local{u16.data(), static_cast<size_t>(len) * 2};
    iovec remote{reinterpret_cast<void*>(str + 0x14), static_cast<size_t>(len) * 2};
    if (process_vm_readv(getpid(), &local, 1, &remote, 1, 0) != static_cast<ssize_t>(len * 2))
        return {};
    std::string out;
    out.reserve(static_cast<size_t>(len));
    for (char16_t c : u16) {
        if (c < 128) out.push_back(static_cast<char>(c));
        else out.push_back('?');
    }
    return out;
}

std::string read_name(uintptr_t player) {
    const uintptr_t ph = mem::read_ptr(player + off::player::kPhoton);
    if (!ph) return {};
    const uintptr_t name = mem::read_ptr(ph + off::photon::kName);
    return read_il2cpp_string(name);
}

void collect_players(uintptr_t manager, uintptr_t local, std::vector<PlayerSnap>& out) {
    out.clear();
    if (!manager) return;

    const uintptr_t list = mem::read_ptr(manager + off::mgr::kList);
    int size = mem::read<int>(manager + off::mgr::kListSize, 0);
    if (list) {
        const int sz2 = mem::read<int>(list + 0x18, 0); // Unity List._size often 0x18
        if (sz2 > 0 && sz2 < 64) size = sz2;
    }
    if (size <= 0 || size > 64) {
        // try list+0x20 as Academic list_size on manager already tried
        size = mem::read<int>(list + off::mgr::kListSize, 0);
    }
    if (size <= 0 || size > 64 || !list) return;

    // Prefer Unity List: _items @ 0x10
    uintptr_t items = mem::read_ptr(list + 0x10);
    if (!items) items = mem::read_ptr(list + off::list::kBuffer);
    if (!items) return;

    // Il2Cpp array: first element @ +0x20
    for (int i = 0; i < size; ++i) {
        uintptr_t pl = mem::read_ptr(items + 0x20 + static_cast<uintptr_t>(i) * 8);
        if (!pl) {
            // Academic entry/stride layout
            pl = mem::read_ptr(items + off::list::kEntry + static_cast<uintptr_t>(i) * off::list::kStride);
        }
        if (!pl) continue;

        PlayerSnap snap;
        snap.addr = pl;
        snap.is_local = (pl == local);
        snap.team = static_cast<int>(mem::read<uint8_t>(pl + off::player::kTeam, 0xFF));
        snap.name = read_name(pl);

        if (!read_bone_pos(pl, off::biped::kHip, snap.feet))
            read_feet_fallback(pl, snap.feet);
        if (!read_bone_pos(pl, off::biped::kHead, snap.head)) {
            snap.head = snap.feet;
            snap.head.y += 1.7f;
        }
        out.push_back(std::move(snap));
    }
}

} // namespace

bool game_init() {
    return true;
}

void game_tick(GameState& st) {
    st.frame++;
    if (!st.il2cpp) {
        st.il2cpp = module_base("libil2cpp.so");
        if (!st.il2cpp) {
            st.status = "wait-il2cpp";
            st.ready = false;
            return;
        }
        NOVA_LOG("libil2cpp @ %p", reinterpret_cast<void*>(st.il2cpp));
    }

    st.manager = resolve_manager(st.il2cpp);
    if (!st.manager) {
        st.status = "no-manager";
        st.ready = false;
        st.players.clear();
        return;
    }

    st.local = mem::read_ptr(st.manager + off::mgr::kLocal);
    st.has_matrix = read_matrix(st.local, st.view_proj);
    collect_players(st.manager, st.local, st.players);
    st.ready = true;
    st.status = st.has_matrix ? "ok" : "no-matrix";
}

bool world_to_screen(const Mat4& vp, const Vec3& world, float sw, float sh, float& out_x, float& out_y) {
    // Unity column-major VP * vec4(world,1)
    const float* m = vp.m;
    const float x = world.x, y = world.y, z = world.z;
    const float clip_x = m[0] * x + m[4] * y + m[8] * z + m[12];
    const float clip_y = m[1] * x + m[5] * y + m[9] * z + m[13];
    const float clip_w = m[3] * x + m[7] * y + m[11] * z + m[15];
    if (clip_w <= 0.001f) return false;
    const float ndc_x = clip_x / clip_w;
    const float ndc_y = clip_y / clip_w;
    out_x = (ndc_x + 1.0f) * 0.5f * sw;
    out_y = (1.0f - ndc_y) * 0.5f * sh;
    return out_x >= -50 && out_x <= sw + 50 && out_y >= -50 && out_y <= sh + 50;
}
