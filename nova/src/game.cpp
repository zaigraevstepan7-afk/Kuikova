#include "game.hpp"
#include "offsets.hpp"
#include "mem.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/uio.h>

#include "stealth.hpp"

namespace {

bool finite4(const Mat4& m) {
    for (int i = 0; i < 16; ++i) {
        if (!std::isfinite(m.m[i])) return false;
    }
    // Reject identity-ish / zero matrices
    float sum = 0;
    for (int i = 0; i < 16; ++i) sum += std::fabs(m.m[i]);
    return sum > 0.01f;
}

uintptr_t module_base(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        // Prefer executable mapping (real ELF base)
        if (line.find("r-xp") == std::string::npos) continue;
        uintptr_t start = 0;
        if (sscanf(line.c_str(), "%lx-", &start) == 1 && start) {
            if (!best || start < best) best = start;
        }
    }
    return best;
}

uintptr_t resolve_manager(uintptr_t il2cpp) {
    // AcademicDLC: TypeInfo +0x90 -> +0x10 -> +0x0
    const uintptr_t ti = il2cpp + off::kPlayerManagerTI;
    const uintptr_t sf = mem::read_ptr(ti + off::mgr::kStaticFields);
    if (!sf) return 0;
    const uintptr_t p2 = mem::read_ptr(sf + off::mgr::kPtr2);
    if (!p2) return 0;
    uintptr_t inst = mem::read_ptr(p2 + off::mgr::kPtr3);
    if (inst) return inst;
    // Fallback: static_fields[0] sometimes is the instance
    return mem::read_ptr(sf);
}

bool read_matrix_from_pmc(uintptr_t pmc, Mat4& out) {
    if (!pmc) return false;
    // AcademicDLC camera nest: +0x20 -> +0x10 -> matrix @ +0xF0
    const uintptr_t t = mem::read_ptr(pmc + off::cam::kTransform);
    if (!t) return false;
    const uintptr_t native = mem::read_ptr(t + off::cam::kPtr);
    if (!native) return false;

    Mat4 m{};
    if (mem::read_into(native + off::cam::kMatrix, m) && finite4(m)) {
        out = m;
        return true;
    }
    // 0.38 public dumps used 0x100 — try as fallback only
    if (mem::read_into(native + 0x100, m) && finite4(m)) {
        out = m;
        return true;
    }
    return false;
}

bool read_matrix(uintptr_t local, Mat4& out) {
    if (!local) return false;

    // Primary: player.main_camera (0xE8) → nest
    const uintptr_t pmc = mem::read_ptr(local + off::player::kMainCamera);
    if (read_matrix_from_pmc(pmc, out)) return true;

    // Holder is on PLAYER at 0x28 (AcademicDLC main_camera_holder), not a substitute for pmc
    const uintptr_t holder = mem::read_ptr(local + off::player::kMainCameraHolder);
    if (holder && holder != pmc) {
        // Holder may wrap the camera component
        if (read_matrix_from_pmc(holder, out)) return true;
        const uintptr_t nested = mem::read_ptr(holder + off::player::kMainCameraHolder);
        if (read_matrix_from_pmc(nested, out)) return true;
    }
    return false;
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
    if (xf && read_transform_pos(xf, out)) return true;
    // Some builds store Transform directly on bone
    return read_transform_pos(bone, out);
}

bool read_feet_fallback(uintptr_t player, Vec3& out) {
    const uintptr_t mov = mem::read_ptr(player + off::player::kMovement);
    if (!mov) return false;
    const uintptr_t data = mem::read_ptr(mov + 0xB0);
    if (!data) return false;
    return mem::read_into(data + off::xform::kPosition, out);
}

std::string read_il2cpp_string(uintptr_t str) {
    if (!str) return {};
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
        if (c >= 32 && c < 127) out.push_back(static_cast<char>(c));
        else if (c >= 128) out.push_back('?');
    }
    return out;
}

std::string read_name(uintptr_t player) {
    const uintptr_t ph = mem::read_ptr(player + off::player::kPhoton);
    if (!ph) return {};
    return read_il2cpp_string(mem::read_ptr(ph + off::photon::kName));
}

bool looks_like_player(uintptr_t pl) {
    if (!pl || pl < 0x10000) return false;
    // team byte should be small enum
    const uint8_t team = mem::read<uint8_t>(pl + off::player::kTeam, 0xFF);
    return team <= 4;
}

void push_player(uintptr_t pl, uintptr_t local, std::vector<PlayerSnap>& out) {
    if (!looks_like_player(pl)) return;
    for (const auto& e : out) if (e.addr == pl) return;

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
    // Skip garbage positions
    if (!std::isfinite(snap.feet.x) || !std::isfinite(snap.head.y)) return;
    out.push_back(std::move(snap));
}

void collect_players(uintptr_t manager, uintptr_t local, std::vector<PlayerSnap>& out) {
    out.clear();
    if (!manager) return;

    const uintptr_t list = mem::read_ptr(manager + off::mgr::kList);
    // AcademicDLC: list_size on manager @ 0x20
    int size = mem::read<int>(manager + off::mgr::kListSize, 0);
    if (size <= 0 || size > 64) {
        if (list) size = mem::read<int>(list + 0x18, 0); // Unity List._size
    }
    if (size <= 0 || size > 64 || !list) return;

    // Path A — AcademicDLC: buffer @ list+0x18, entries at +0x30 stride 0x18
    const uintptr_t acad_buf = mem::read_ptr(list + off::list::kBuffer);
    if (acad_buf) {
        for (int i = 0; i < size; ++i) {
            const uintptr_t pl = mem::read_ptr(
                acad_buf + off::list::kEntry + static_cast<uintptr_t>(i) * off::list::kStride);
            push_player(pl, local, out);
        }
        if (!out.empty()) return;
    }

    // Path B — Unity List<T>: _items @ 0x10, array elems @ +0x20
    uintptr_t items = mem::read_ptr(list + 0x10);
    if (!items) items = acad_buf;
    if (!items) return;
    for (int i = 0; i < size; ++i) {
        const uintptr_t pl = mem::read_ptr(items + 0x20 + static_cast<uintptr_t>(i) * 8);
        push_player(pl, local, out);
    }
}

} // namespace

bool game_init() { return true; }

void game_tick(GameState& st) {
    st.frame++;
    // Throttle heavy work slightly
    if ((st.frame & 1) == 0 && st.ready) {
        // still refresh matrix every frame when ready — fall through every frame for matrix
    }

    if (!st.il2cpp) {
        st.il2cpp = module_base(XS("libil2cpp.so"));
        if (!st.il2cpp) {
            st.status = "wait";
            st.ready = false;
            return;
        }
    }

    st.manager = resolve_manager(st.il2cpp);
    if (!st.manager) {
        st.status = "lobby";
        st.ready = false;
        st.players.clear();
        st.has_matrix = false;
        return;
    }

    st.local = mem::read_ptr(st.manager + off::mgr::kLocal);
    st.has_matrix = read_matrix(st.local, st.view_proj);

    // Collect players every other frame to cut noise
    if ((st.frame & 1) == 1 || st.players.empty())
        collect_players(st.manager, st.local, st.players);

    st.ready = true;
    st.status = st.has_matrix ? "ok" : "cam";
}

bool world_to_screen(const Mat4& vp, const Vec3& world, float sw, float sh, float& out_x, float& out_y) {
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
    return out_x >= -80 && out_x <= sw + 80 && out_y >= -80 && out_y <= sh + 80;
}
