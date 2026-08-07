#include "game.hpp"
#include "offsets.hpp"
#include "mem.hpp"
#include "stealth.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

namespace {

bool finite3(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

bool finite4(const Mat4& m) {
    float sum = 0.f;
    for (int i = 0; i < 16; ++i) {
        if (!std::isfinite(m.m[i])) return false;
        sum += std::fabs(m.m[i]);
    }
    return sum > 0.01f;
}

uintptr_t module_base(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        if (line.find("r-xp") == std::string::npos) continue;
        uintptr_t start = 0;
        if (sscanf(line.c_str(), "%lx-", &start) == 1 && start) {
            if (!best || start < best) best = start;
        }
    }
    return best;
}

uintptr_t chain_manager(uintptr_t sf) {
    if (!sf) return 0;
    // AcademicDLC: static_fields +0x10 -> +0x0
    const uintptr_t p2 = mem::read_ptr(sf + off::mgr::kPtr2);
    if (p2) {
        const uintptr_t inst = mem::read_ptr(p2 + off::mgr::kPtr3);
        if (inst) return inst;
    }
    // Sometimes instance lives at static_fields+0
    return mem::read_ptr(sf);
}

uintptr_t resolve_manager(uintptr_t il2cpp) {
    const uintptr_t ti_loc = il2cpp + off::kPlayerManagerTI;

    // A) RVA points at Il2CppClass object directly (AcademicDLC style: ti+0x90)
    uintptr_t sf = mem::read_ptr(ti_loc + off::mgr::kStaticFields);
    uintptr_t inst = chain_manager(sf);
    if (inst) return inst;

    // B) RVA is a global Il2CppClass* — one dereference first
    const uintptr_t klass = mem::read_ptr(ti_loc);
    if (klass && klass != ti_loc) {
        sf = mem::read_ptr(klass + off::mgr::kStaticFields);
        inst = chain_manager(sf);
        if (inst) return inst;
    }
    return 0;
}

bool read_matrix_at_native(uintptr_t native, Mat4& out) {
    if (!native) return false;
    Mat4 m{};
    // AcademicDLC 0.39.2: matrix @ +0xF0
    if (mem::read_into(native + off::cam::kMatrix, m) && finite4(m)) {
        out = m;
        return true;
    }
    // Older public dumps used 0x100
    if (mem::read_into(native + 0x100, m) && finite4(m)) {
        out = m;
        return true;
    }
    return false;
}

bool read_matrix_from_pmc(uintptr_t pmc, Mat4& out) {
    if (!pmc) return false;
    // nest: +0x20 -> +0x10 -> matrix
    const uintptr_t t = mem::read_ptr(pmc + off::cam::kTransform);
    if (!t) return false;
    const uintptr_t native = mem::read_ptr(t + off::cam::kPtr);
    return read_matrix_at_native(native, out);
}

bool read_matrix(uintptr_t local, Mat4& out) {
    if (!local) return false;

    const uintptr_t pmc = mem::read_ptr(local + off::player::kMainCamera);
    if (read_matrix_from_pmc(pmc, out)) return true;

    // player.main_camera_holder @ 0x28
    const uintptr_t holder = mem::read_ptr(local + off::player::kMainCameraHolder);
    if (holder && holder != pmc) {
        if (read_matrix_from_pmc(holder, out)) return true;
        const uintptr_t nested = mem::read_ptr(holder + off::cam::kTransform);
        if (nested) {
            const uintptr_t native = mem::read_ptr(nested + off::cam::kPtr);
            if (read_matrix_at_native(native, out)) return true;
        }
    }
    return false;
}

bool read_transform_pos(uintptr_t transform, Vec3& out) {
    if (!transform) return false;
    const uintptr_t data = mem::read_ptr(transform + off::xform::kData);
    if (!data) return false;
    Vec3 v{};
    if (!mem::read_into(data + off::xform::kPosition, v)) return false;
    if (!finite3(v)) return false;
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

    // AcademicDLC: bone + transform_object(0x10) -> Transform
    const uintptr_t xf = mem::read_ptr(bone + off::biped::kTransformObject);
    if (xf && read_transform_pos(xf, out)) return true;
    return read_transform_pos(bone, out);
}

bool read_feet_fallback(uintptr_t player, Vec3& out) {
    const uintptr_t mov = mem::read_ptr(player + off::player::kMovement);
    if (!mov) return false;
    // movement.translation_data @ 0xB0 (AcademicDLC)
    const uintptr_t data = mem::read_ptr(mov + 0xB0);
    if (!data) return false;
    Vec3 v{};
    if (!mem::read_into(data + off::xform::kPosition, v)) return false;
    if (!finite3(v)) return false;
    out = v;
    return true;
}

std::string read_il2cpp_string(uintptr_t str) {
    if (!str) return {};
    const int32_t len = mem::read<int32_t>(str + 0x10, 0);
    if (len <= 0 || len > 64) return {};

    // 64-bit Il2CppString chars usually @ 0x14; some builds pad to 0x18
    uintptr_t chars_at = str + 0x14;
    std::vector<char16_t> u16(static_cast<size_t>(len));
    if (!mem::read_bytes(chars_at, u16.data(), static_cast<size_t>(len) * 2)) {
        chars_at = str + 0x18;
        if (!mem::read_bytes(chars_at, u16.data(), static_cast<size_t>(len) * 2))
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

    if (!read_bone_pos(pl, off::biped::kHip, snap.feet)) {
        if (!read_feet_fallback(pl, snap.feet)) return;
    }
    if (!read_bone_pos(pl, off::biped::kHead, snap.head)) {
        snap.head = snap.feet;
        snap.head.y += 1.7f;
    }

    // Sanitize absurd bone distances
    const float dy = std::fabs(snap.head.y - snap.feet.y);
    if (dy < 0.4f || dy > 3.2f) {
        snap.head = snap.feet;
        snap.head.y += 1.7f;
    }
    if (!finite3(snap.feet) || !finite3(snap.head)) return;
    // Skip origin junk
    if (snap.feet.x == 0.f && snap.feet.y == 0.f && snap.feet.z == 0.f) return;

    out.push_back(std::move(snap));
}

void collect_players(uintptr_t manager, uintptr_t local, std::vector<PlayerSnap>& out) {
    out.clear();
    if (!manager) return;

    const uintptr_t list = mem::read_ptr(manager + off::mgr::kList);
    if (!list) return;

    int size = mem::read<int>(manager + off::mgr::kListSize, 0);
    if (size <= 0 || size > 64)
        size = mem::read<int>(list + 0x18, 0); // Unity List._size
    if (size <= 0 || size > 64)
        size = mem::read<int>(list + off::mgr::kListSize, 0);
    if (size <= 0 || size > 64) return;

    // Path A — AcademicDLC custom: buffer @ +0x18, entry 0x30 stride 0x18
    const uintptr_t acad_buf = mem::read_ptr(list + off::list::kBuffer);
    if (acad_buf) {
        for (int i = 0; i < size; ++i) {
            const uintptr_t pl = mem::read_ptr(
                acad_buf + off::list::kEntry +
                static_cast<uintptr_t>(i) * off::list::kStride);
            push_player(pl, local, out);
        }
        if (!out.empty()) return;
    }

    // Path B — Unity List<T>: _items @ 0x10, elems @ array+0x20
    uintptr_t items = mem::read_ptr(list + 0x10);
    if (!items) items = acad_buf;
    if (!items) return;
    for (int i = 0; i < size; ++i) {
        const uintptr_t pl =
            mem::read_ptr(items + 0x20 + static_cast<uintptr_t>(i) * sizeof(uintptr_t));
        push_player(pl, local, out);
    }
}

// Column-major M * vec4
inline bool w2s_col(const float* m, const Vec3& w, float sw, float sh, float& ox, float& oy) {
    const float clip_x = m[0] * w.x + m[4] * w.y + m[8] * w.z + m[12];
    const float clip_y = m[1] * w.x + m[5] * w.y + m[9] * w.z + m[13];
    const float clip_w = m[3] * w.x + m[7] * w.y + m[11] * w.z + m[15];
    if (clip_w <= 0.01f) return false;
    const float inv = 1.0f / clip_w;
    const float ndc_x = clip_x * inv;
    const float ndc_y = clip_y * inv;
    ox = (ndc_x + 1.0f) * 0.5f * sw;
    oy = (1.0f - ndc_y) * 0.5f * sh; // Unity NDC Y up → screen Y down
    return std::isfinite(ox) && std::isfinite(oy);
}

// Row-major fallback (some dumps store transposed)
inline bool w2s_row(const float* m, const Vec3& w, float sw, float sh, float& ox, float& oy) {
    const float clip_x = m[0] * w.x + m[1] * w.y + m[2] * w.z + m[3];
    const float clip_y = m[4] * w.x + m[5] * w.y + m[6] * w.z + m[7];
    const float clip_w = m[12] * w.x + m[13] * w.y + m[14] * w.z + m[15];
    if (clip_w <= 0.01f) return false;
    const float inv = 1.0f / clip_w;
    ox = (clip_x * inv + 1.0f) * 0.5f * sw;
    oy = (1.0f - clip_y * inv) * 0.5f * sh;
    return std::isfinite(ox) && std::isfinite(oy);
}

} // namespace

bool game_init() { return true; }

void game_tick(GameState& st) {
    st.frame++;

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

    // Refresh players every frame when matrix is live (ESP smoothness)
    if (st.has_matrix || (st.frame & 1))
        collect_players(st.manager, st.local, st.players);

    st.ready = true;
    if (!st.has_matrix) st.status = "cam";
    else if (st.players.empty()) st.status = "nop";
    else st.status = "ok";
}

bool world_to_screen(const Mat4& vp, const Vec3& world, float sw, float sh, float& out_x, float& out_y) {
    if (sw < 1.f || sh < 1.f) return false;
    if (!finite3(world)) return false;

    float x = 0, y = 0;
    if (w2s_col(vp.m, world, sw, sh, x, y)) {
        // Accept on-screen or slightly off (box edges)
        if (x >= -sw && x <= sw * 2.f && y >= -sh && y <= sh * 2.f) {
            out_x = x;
            out_y = y;
            return true;
        }
    }
    if (w2s_row(vp.m, world, sw, sh, x, y)) {
        if (x >= -sw && x <= sw * 2.f && y >= -sh && y <= sh * 2.f) {
            out_x = x;
            out_y = y;
            return true;
        }
    }
    return false;
}
