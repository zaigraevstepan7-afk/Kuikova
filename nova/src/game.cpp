#include "game.hpp"
#include "offsets.hpp"
#include "mem.hpp"
#include "module_base.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

namespace {

bool finite3(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

bool matrix_nonzero(const Mat4& m) {
    return m.m[0] != 0.f || m.m[5] != 0.f || m.m[10] != 0.f || m.m[15] != 0.f ||
           m.m[1] != 0.f || m.m[2] != 0.f;
}

// Melodium: reject pure view (last row ~ 0,0,0,1) — need VP for boxes
bool looks_like_vp(const Mat4& m) {
    if (!matrix_nonzero(m)) return false;
    // row-major last row indices 12..15 in Melodium struct = our m[12..15] if same pack
    // For float[16] column-major Unity: last row is m[3],m[7],m[11],m[15]
    // Melodium Matrix packs as m00,m01,m02,m03,... so last row m30..m33 at bytes 48-63 = indices 12-15
    // They memcpy from game memory the same 64 bytes — so use their check on indices 12-15:
    const bool last_row_id =
        m.m[12] == 0.f && m.m[13] == 0.f && m.m[14] == 0.f &&
        (m.m[15] == 1.f || m.m[15] == 0.f);
    return !last_row_id;
}

uintptr_t chain_manager_from_klass(uintptr_t klass) {
    if (!klass) return 0;
    const uintptr_t sf = mem::read_ptr(klass + off::mgr::kStaticFields); // 0x90
    if (!sf) return 0;
    // Community: +0x10 then +0x0
    uintptr_t inst = mem::read_ptr(sf + off::mgr::kPtr2); // 0x10
    if (inst) {
        // sometimes 0x10 already is instance; sometimes need +0
        const uintptr_t via0 = mem::read_ptr(inst + off::mgr::kPtr3);
        if (via0) return via0;
        return inst;
    }
    return mem::read_ptr(sf + off::mgr::kPtr3); // +0
}

uintptr_t resolve_manager(uintptr_t il2cpp) {
    // Melodium/Halalium: TypeInfo RVA is POINTER to Il2CppClass*
    //   klass = *(base + TypeInfo)
    const uintptr_t ti_loc = il2cpp + off::kPlayerManagerTI;

    const uintptr_t klass = mem::read_ptr(ti_loc);
    if (klass) {
        if (uintptr_t inst = chain_manager_from_klass(klass))
            return inst;
    }
    // Fallback: treat as embedded class (rare)
    return chain_manager_from_klass(ti_loc);
}

bool try_matrix_at(uintptr_t base, uintptr_t off, Mat4& out) {
    Mat4 m{};
    if (!base || !mem::read_into(base + off, m)) return false;
    if (!matrix_nonzero(m)) return false;
    out = m;
    return true;
}

bool read_matrix(uintptr_t local, Mat4& out) {
    if (!local) return false;
    // Melodium: PlayerMainCamera = local+0xE8
    // nest: +0x20 → +0x10 → matrix @ 0xF0 / 0x100
    const uintptr_t pmc = mem::read_ptr(local + off::player::kMainCamera);
    if (!pmc) return false;

    const uintptr_t a = mem::read_ptr(pmc + 0x20);
    if (a) {
        const uintptr_t b = mem::read_ptr(a + 0x10);
        if (b) {
            if (try_matrix_at(b, 0xF0, out) || try_matrix_at(b, 0x100, out))
                return true;
        }
    }

    // Holder path: local+0x28
    const uintptr_t holder = mem::read_ptr(local + off::player::kMainCameraHolder);
    if (holder) {
        const uintptr_t a2 = mem::read_ptr(holder + 0x20);
        if (a2) {
            const uintptr_t b2 = mem::read_ptr(a2 + 0x10);
            if (b2 && (try_matrix_at(b2, 0xF0, out) || try_matrix_at(b2, 0x100, out)))
                return true;
        }
    }
    return false;
}

bool read_transform_pos(uintptr_t transform, Vec3& out) {
    if (!transform) return false;
    const uintptr_t data = mem::read_ptr(transform + off::xform::kData);
    if (!data) return false;
    Vec3 v{};
    if (!mem::read_into(data + off::xform::kPosition, v) || !finite3(v)) return false;
    out = v;
    return true;
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
    return read_transform_pos(bone, out);
}

bool read_feet_fallback(uintptr_t player, Vec3& out) {
    const uintptr_t mov = mem::read_ptr(player + off::player::kMovement);
    if (!mov) return false;
    const uintptr_t data = mem::read_ptr(mov + 0xB0);
    if (!data) return false;
    Vec3 v{};
    if (!mem::read_into(data + off::xform::kPosition, v) || !finite3(v)) return false;
    out = v;
    return true;
}

std::string read_il2cpp_string(uintptr_t str) {
    if (!str) return {};
    const int32_t len = mem::read<int32_t>(str + 0x10, 0);
    if (len <= 0 || len > 64) return {};
    std::vector<char16_t> u16(static_cast<size_t>(len));
    if (!mem::read_bytes(str + 0x14, u16.data(), static_cast<size_t>(len) * 2)) {
        if (!mem::read_bytes(str + 0x18, u16.data(), static_cast<size_t>(len) * 2))
            return {};
    }
    std::string out;
    out.reserve(static_cast<size_t>(len));
    for (char16_t c : u16) {
        if (c >= 32 && c < 127) out.push_back(static_cast<char>(c));
        else if (c > 127) out.push_back('?');
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
    const uint8_t team = mem::read<uint8_t>(pl + off::player::kTeam, 0xFF);
    if (team <= 8) return true;
    return mem::read_ptr(pl + off::player::kMovement) ||
           mem::read_ptr(pl + off::player::kPhoton) ||
           mem::read_ptr(pl + off::player::kCharacterView);
}

void push_player(uintptr_t pl, uintptr_t local, std::vector<PlayerSnap>& out) {
    if (!looks_like_player(pl)) return;
    for (const auto& e : out) if (e.addr == pl) return;

    PlayerSnap snap;
    snap.addr = pl;
    snap.is_local = (pl == local);
    snap.team = static_cast<int>(mem::read<uint8_t>(pl + off::player::kTeam, 0xFF));
    snap.name = read_name(pl);

    if (!read_bone_pos(pl, off::biped::kHip, snap.feet)) {
        if (!read_feet_fallback(pl, snap.feet)) return;
    }
    if (!read_bone_pos(pl, off::biped::kHead, snap.head)) {
        snap.head = snap.feet;
        snap.head.y += 1.7f;
    }
    const float dy = std::fabs(snap.head.y - snap.feet.y);
    if (dy < 0.4f || dy > 3.2f) {
        snap.head = snap.feet;
        snap.head.y += 1.7f;
    }
    if (!finite3(snap.feet) || !finite3(snap.head)) return;
    if (snap.feet.x == 0.f && snap.feet.y == 0.f && snap.feet.z == 0.f) return;
    out.push_back(std::move(snap));
}

void collect_players(uintptr_t manager, uintptr_t local, std::vector<PlayerSnap>& out) {
    out.clear();
    if (!manager) return;

    const uintptr_t list = mem::read_ptr(manager + off::mgr::kList);
    int size = mem::read<int>(manager + off::mgr::kListSize, 0);
    if (list) {
        int sz = mem::read<int>(list + 0x18, 0);
        if (sz > 0 && sz <= 64) size = sz;
        else {
            sz = mem::read<int>(list + 0x20, 0);
            if (sz > 0 && sz <= 64) size = sz;
        }
    }
    if (size <= 0 || size > 64) size = 16;

    if (list) {
        const uintptr_t buf = mem::read_ptr(list + off::list::kBuffer); // 0x18
        const uintptr_t items = mem::read_ptr(list + 0x10);
        if (buf) {
            for (int i = 0; i < size; ++i)
                push_player(mem::read_ptr(buf + off::list::kEntry +
                                          (uintptr_t)i * off::list::kStride), local, out);
        }
        if (items) {
            for (int i = 0; i < size; ++i)
                push_player(mem::read_ptr(items + 0x20 + (uintptr_t)i * 8), local, out);
        }
        if (out.empty() && buf) {
            for (int i = 0; i < size; ++i)
                push_player(mem::read_ptr(buf + 0x20 + (uintptr_t)i * 8), local, out);
        }
    }
    // alt local @ 0x68 (Melodium)
    if (!local) local = mem::read_ptr(manager + 0x68);
    if (local) push_player(local, local, out);
}

} // namespace

bool game_init() { return true; }

void game_tick(GameState& st) {
    st.frame++;

    if (!st.il2cpp) {
        st.il2cpp = mods::resolve_il2cpp();
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
    if (!st.local) st.local = mem::read_ptr(st.manager + 0x68);

    Mat4 mat{};
    st.has_matrix = read_matrix(st.local, mat);
    if (st.has_matrix) {
        // Prefer VP-looking matrix; still keep if only nonzero
        if (looks_like_vp(mat) || matrix_nonzero(mat))
            st.view_proj = mat;
        else
            st.has_matrix = false;
    }

    collect_players(st.manager, st.local, st.players);

    st.ready = true;
    if (!st.local) st.status = "nolocal";
    else if (!st.has_matrix) st.status = "cam";
    else if (st.players.size() <= 1) st.status = "alone";
    else st.status = "ok";
}

// Melodium world2screen (proven) — treats float[16] as their Matrix field order
bool world_to_screen(const Mat4& vp, const Vec3& world, float sw, float sh, float& out_x, float& out_y) {
    if (sw < 1.f || sh < 1.f || !finite3(world)) return false;
    const float* m = vp.m;
    // Melodium: clip = row-style against packed floats
    const float clipX = world.x * m[0] + world.y * m[1] + world.z * m[2] + m[3];
    const float clipY = world.x * m[4] + world.y * m[5] + world.z * m[6] + m[7];
    const float clipW = world.x * m[12] + world.y * m[13] + world.z * m[14] + m[15];
    if (clipW < 0.001f) return false;
    const float ndcX = clipX / clipW;
    const float ndcY = clipY / clipW;
    out_x = (ndcX * 0.5f + 0.5f) * sw;
    out_y = (1.f - (ndcY * 0.5f + 0.5f)) * sh;
    return std::isfinite(out_x) && std::isfinite(out_y);
}
