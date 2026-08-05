#include "esp.h"
#include <imgui.h>
#include <map>
#include <algorithm>
#include <cstdio>
#include <src/menu/gui.h>
#include "sdk/OffsetsBridge.h"
#include "includes/halalium_mem.h"
#include "includes/halalium_chains.h"

Matrix esp::matrix()
{
    // EGL only consumes Unity-thread snapshot — never LDR camera here
    if (m_have_matrix)
        return m_cached;
    return Matrix{};
}

static bool matrix_nonzero(const Matrix &m)
{
    return m.m00 != 0.f || m.m11 != 0.f || m.m22 != 0.f || m.m33 != 0.f;
}

// world2screen() needs view-projection (clip + perspective divide).
// Pure worldToCamera is usually last-row [0,0,0,1] and will blank ESP.
static bool looks_like_vp(const Matrix &m)
{
    if (!matrix_nonzero(m))
        return false;
    const bool last_row_identity =
        m.m30 == 0.f && m.m31 == 0.f && m.m32 == 0.f &&
        (m.m33 == 1.f || m.m33 == 0.f);
    return !last_row_identity;
}

static bool try_read_matrix_at(uintptr_t base, uintptr_t off, Matrix &out)
{
    return base && hmem::read(base + off, out) && matrix_nonzero(out);
}

static bool try_read_vp_at(uintptr_t base, uintptr_t off, Matrix &out)
{
    return try_read_matrix_at(base, off, out) && looks_like_vp(out);
}

// Unity thread ONLY.
// Melodium/SO2 W2S nest (proven with this world2screen):
//   PlayerMainCamera+0x20 → +0x10 → matrix@0xF0 (also try 0x100)
// Halalium E8→28→30 is Unity Camera* (FOV); use it for P*V Injected product.
void esp::cache_matrix()
{
    Matrix mat{};
    bool ok = false;
    int stage = 0;

    void *player = (c_player && c_player->local) ? (void *)c_player->local : nullptr;
    if (player)
    {
        const uintptr_t pmc = hmem::read_ptr(reinterpret_cast<uintptr_t>(player) + Offsets::Player::main_camera);
        if (pmc)
        {
            stage = 1;

            // 1) Melodium community nest — primary for boxes (accept any nonzero @0xF0/0x100)
            const uintptr_t a = hmem::read_ptr(pmc + 0x20);
            if (a)
            {
                stage = 2;
                const uintptr_t b = hmem::read_ptr(a + 0x10);
                if (b)
                {
                    stage = 3;
                    if (try_read_matrix_at(b, 0xF0, mat) || try_read_matrix_at(b, 0x100, mat))
                    {
                        ok = true;
                        stage = 5;
                    }
                }
            }

            // 2) Halalium Unity Camera nest → projection * worldToCamera
            if (!ok)
            {
                const uintptr_t nested = hmem::read_ptr(pmc + Offsets::PlayerMainCamera::nested);
                if (nested)
                {
                    if (stage < 2)
                        stage = 2;
                    const uintptr_t ucam = hmem::read_ptr(nested + Offsets::PlayerMainCamera::unity_camera);
                    if (ucam)
                    {
                        stage = 3;
                        Matrix w2c{};
                        Matrix proj{};
                        if (c_fn && c_fn->get_w2c_injected)
                            c_fn->get_w2c_injected(reinterpret_cast<void *>(ucam), &w2c);
                        if (c_fn && c_fn->get_proj_injected)
                            c_fn->get_proj_injected(reinterpret_cast<void *>(ucam), &proj);
                        if (matrix_nonzero(w2c) && matrix_nonzero(proj))
                        {
                            mat = proj * w2c;
                            if (matrix_nonzero(mat))
                            {
                                ok = true;
                                stage = 5;
                            }
                        }
                        if (!ok && (try_read_vp_at(ucam, Offsets::Camera::matrix, mat) ||
                                    try_read_vp_at(ucam, 0x100, mat)))
                        {
                            ok = true;
                            stage = 5;
                        }
                        if (!ok)
                        {
                            const uintptr_t native = hmem::read_ptr(ucam + 0x10);
                            if (try_read_vp_at(native, Offsets::Camera::matrix, mat) ||
                                try_read_vp_at(native, 0x100, mat))
                            {
                                ok = true;
                                stage = 5;
                            }
                        }
                        if (!ok)
                            stage = 4;
                    }
                }
            }
        }
    }

    m_cached = mat;
    m_have_matrix = ok;
    dbg_matrix.store(ok ? 1 : 0, std::memory_order_relaxed);
    dbg_local.store(player ? 1 : 0, std::memory_order_relaxed);
    dbg_stage.store(stage, std::memory_order_relaxed);
}

// Snapshot world positions on Unity thread (Update/LateUpdate).
// Always update dbg_* so status bar shows whether memory reads work.
void esp::snapshot()
{
    std::vector<EspSnap> next;
    Vector3 cam{};
    int n_players = 0;
    int n_enemies = 0;

    if (c_player)
    {
        n_players = (int)c_player->entity.size();
        dbg_local.store(c_player->local ? 1 : 0, std::memory_order_relaxed);
    }
    else
    {
        dbg_local.store(0, std::memory_order_relaxed);
    }

    if (!c_player || !c_player->local || !c_globals)
    {
        std::lock_guard<std::mutex> lock(m_snap_mu);
        m_snap.clear();
        m_cam_pos = {};
        dbg_players.store(n_players, std::memory_order_relaxed);
        dbg_enemies.store(0, std::memory_order_relaxed);
        dbg_snap.store(0, std::memory_order_relaxed);
        return;
    }

    if (c_player->local->m_pMainCameraHolder)
        cam = c_player->local->m_pMainCameraHolder->get_position();

    next.reserve(c_player->entity.size());

    // First pass: count how many non-local pass team filter
    int team_ok = 0;
    for (int i = 0; i < (int)c_player->entity.size(); ++i)
    {
        c_player_controller *player = c_player->entity[i];
        if (!player || player == c_player->local)
            continue;
        if (!c_globals->is_allocated(player))
            continue;
        const uint8_t tl = hchain::team(c_player->local);
        const uint8_t te = hchain::team(player);
        if ((tl != te) || (tl == 0 && te == 0))
            ++team_ok;
    }
    // If team bytes look broken (everyone same / zero), draw all non-local
    const bool soft_team = (team_ok == 0);

    for (int i = 0; i < (int)c_player->entity.size(); ++i)
    {
        c_player_controller *player = c_player->entity[i];
        if (!player || player == c_player->local)
            continue;
        if (!c_globals->is_allocated(player))
            continue;

        const uint8_t tl = hchain::team(c_player->local);
        const uint8_t te = hchain::team(player);
        const bool enemy = soft_team || (tl != te) || (tl == 0 && te == 0);
        if (!enemy)
            continue;
        ++n_enemies;

        if (!g.b_esp)
            continue;

        void *ph = hchain::photon(player);
        c_photon_player *photon = reinterpret_cast<c_photon_player *>(ph);
        // LDR transform @ Player+0x100
        c_transform *transform = reinterpret_cast<c_transform *>(
            hmem::read_ptr(reinterpret_cast<uintptr_t>(player) + 0x100));
        if (!photon || !transform)
        {
            // struct fallback
            photon = player->m_pPhoton;
            transform = player->m_pTransform;
        }
        if (!photon || !transform)
            continue;

        EspSnap s{};
        s.player = player;
        s.foot = transform->get_position(); // Unity thread — OK
        if (s.foot == Vector3{})
            continue;

        const Vector3 base = s.foot;
        s.foot = {base.x, base.y - 0.1f, base.z};
        s.head = {base.x, base.y + 1.7f, base.z};

        s.health = photon->get_health();
        if (s.health < 0)
            s.health = 100;
        s.name = photon->m_nameField;
        next.push_back(s);
    }

    std::lock_guard<std::mutex> lock(m_snap_mu);
    m_snap.swap(next);
    m_cam_pos = cam;
    dbg_players.store(n_players, std::memory_order_relaxed);
    dbg_enemies.store(n_enemies, std::memory_order_relaxed);
    dbg_snap.store((int)m_snap.size(), std::memory_order_relaxed);
}

void esp::draw_status()
{
    // Always-on diagnostic strip — top + bottom, FG + BG (never gated by ESP/SDK)
    ImGuiIO &io = ImGui::GetIO();
    float w = io.DisplaySize.x;
    float h = io.DisplaySize.y;
    if (c_egl && c_egl->width > 0 && c_egl->heigth > 0)
    {
        w = (float)c_egl->width;
        h = (float)c_egl->heigth;
    }
    if (w < 1.f || h < 1.f)
        return;

    static const char *stages[] = {"no-local", "pmc", "nest", "ucam", "mat-fail", "mat-ok"};
    int st = dbg_stage.load(std::memory_order_relaxed);
    if (st < 0 || st > 5)
        st = 0;

    static const char *sdk_stages[] = {
        "idle", "bases", "no-il2cpp", "bind", "wait-asm", "api-fail", "partial", "hooks", "ready"};
    int ss = dbg_sdk_stage.load(std::memory_order_relaxed);
    if (ss < 0 || ss > 8)
        ss = 0;

    static int frame = 0;
    ++frame;

    char buf[320]{};
    std::snprintf(buf, sizeof(buf),
                  "xxx#4 | sdk:%s(%s) | local:%s | matrix:%s(%s) | players:%d | enemies:%d | snap:%d | f:%d",
                  dbg_sdk.load(std::memory_order_relaxed) ? "OK" : "NO",
                  sdk_stages[ss],
                  dbg_local.load(std::memory_order_relaxed) ? "OK" : "NO",
                  dbg_matrix.load(std::memory_order_relaxed) ? "OK" : "NO",
                  stages[st],
                  dbg_players.load(std::memory_order_relaxed),
                  dbg_enemies.load(std::memory_order_relaxed),
                  dbg_snap.load(std::memory_order_relaxed),
                  frame);

    auto paint = [&](ImDrawList *dl, float y0) {
        if (!dl)
            return;
        const float bar_h = 32.f;
        dl->AddRectFilled(ImVec2(0, y0), ImVec2(w, y0 + bar_h), IM_COL32(0, 0, 0, 240));
        dl->AddRect(ImVec2(0, y0), ImVec2(w, y0 + bar_h), IM_COL32(255, 220, 60, 255), 0.f, 0, 2.f);
        ImFont *font = gui::font ? gui::font : ImGui::GetFont();
        if (font)
            dl->AddText(font, 16.f, ImVec2(8.f, y0 + 7.f), IM_COL32(255, 240, 120, 255), buf);
        else
            dl->AddText(ImVec2(8.f, y0 + 7.f), IM_COL32(255, 240, 120, 255), buf);
    };

    ImDrawList *fg = ImGui::GetForegroundDrawList();
    ImDrawList *bg = ImGui::GetBackgroundDrawList();
    paint(fg, 0.f);
    paint(bg, 0.f);
    paint(fg, h - 32.f);
    paint(bg, h - 32.f);
}

bool esp::update_matrix()
{
    cache_matrix();
    return m_have_matrix;
}

void esp::clear_matrix()
{
    m_cached = Matrix{};
    m_have_matrix = false;
    std::lock_guard<std::mutex> lock(m_snap_mu);
    m_snap.clear();
    m_cam_pos = {};
    dbg_matrix.store(0, std::memory_order_relaxed);
    dbg_snap.store(0, std::memory_order_relaxed);
    dbg_enemies.store(0, std::memory_order_relaxed);
}

void text(ImFont *font, float FontSize, const ImVec2 &position, const ImColor &textColor, const char *text, bool outline, bool shadow)
{
    ImColor outlineColor(0.f, 0.0f, 0.0f, 0.7f);
    float outlineThickness = 1.0f;
    auto drawList = ImGui::GetBackgroundDrawList();
    if (outline)
    {
        const float offset[] = {
            -outlineThickness, -outlineThickness, 0, outlineThickness, outlineThickness};

        for (float xOffset : offset)
        {
            for (float yOffset : offset)
            {
                drawList->AddText(font, FontSize, ImVec2(position.x + xOffset, position.y + yOffset), outlineColor, text);
            }
        }
    }

    if (shadow)
    {
        drawList->AddText(font, FontSize, {position.x + outlineThickness, position.y + outlineThickness}, ImColor(0, 0, 0, 255), text);
    }

    drawList->AddText(font, FontSize, position, textColor, text);
}

void DrawLine(const ImVec2 &ot, const ImVec2 &kuda, const ImVec4 &color, bool shadow, bool glow)
{
    if (color.w == 0.0f)
        return;

    auto draw = ImGui::GetBackgroundDrawList();
    if (shadow)
        draw->AddLine(ot, kuda, IM_COL32(0, 0, 0, 255), 2);
    if (glow)
        for (unsigned char i = 0; i < 0; ++i)
            (void)i; // intentionally disabled glow loop (was div-by-zero)

    draw->AddLine(ot, kuda, ImGui::ColorConvertFloat4ToU32(color), 1);
}

void DrawCircle(const ImVec2 &ot, float radius, const ImVec4 &color, bool shadow, bool glow)
{
    if (color.w == 0.0f)
        return;

    auto draw = ImGui::GetBackgroundDrawList();
    if (shadow)
        draw->AddCircle(ot, radius, IM_COL32(0, 0, 0, 255), 0, 2);
    if (glow)
        for (unsigned char i = 0; i < 0; ++i)
            (void)i; // intentionally disabled glow loop (was div-by-zero)

    draw->AddCircle(ot, radius, ImGui::ColorConvertFloat4ToU32(color), 0, 1);
}

void DrawSkeleton(const ImVec4 &color, bool shadow, bool glow, bool DrawHead, Matrix worldToCameraMatrix, c_player_controller *m_pController)
{
    if (color.w == 0.0f)
        return;

    auto m_pCharacterView = m_pController->m_pCharacterView;
    if (!m_pCharacterView)
        return;

    auto _bipedMap = m_pCharacterView->c_biped;
    if (!_bipedMap)
        return;

    if (!_bipedMap->head || !_bipedMap->spine1)
        return;
    if (!c_globals->is_allocated(_bipedMap->head) || !c_globals->is_allocated(_bipedMap->spine1))
        return;

    auto headpos = _bipedMap->head->get_position();
    auto head = c_globals->world2screen(worldToCameraMatrix, headpos);
    auto spine1 = c_globals->world2screen(worldToCameraMatrix, _bipedMap->spine1->get_position());
    if (head.z < 0 || spine1.z < 0)
        return;

    std::array<ImVec2, 22> array{ImVec2(head.x, head.y)};
    const unsigned short start = 0x28;

    for (unsigned short i = start; i <= 0xC8; i += 0x8)
    {
        unsigned short index = (i - start) / 0x8;
        if (i == 0x30 || i == 0x38)
        {
            array[index + 1] = array[index];
            continue;
        }
        auto a = *(c_transform **)((uintptr_t)_bipedMap + i);
        if (!a || !c_globals->is_allocated(a))
            continue;
        auto pos = c_globals->world2screen(worldToCameraMatrix, a->get_position());
        if (pos.z < 0.125f)
            continue;

        ImVec2 current(pos.x, pos.y);
        ImVec2 prev = array[index];
        if (i == 0x68 || i == 0x88)
        {
            prev = array[4];
        }
        if (i == 0xB0)
        {
            prev = array[13];
        }

        float dx = current.x - prev.x;
        float dy = current.y - prev.y;
        float length = sqrtf(dx * dx + dy * dy);
        if (length > 0.0f)
        {
            float stepX = dx / length * 1.0f;
            float stepY = dy / length * 1.0f;
            float t = 0.0f;
            while (t < length)
            {
                ImVec2 start(prev.x + stepX * t, prev.y + stepY * t);
                ImVec2 end(prev.x + stepX * (t + 1.0f), prev.y + stepY * (t + 1.0f));
                DrawLine(start, end, color, shadow, glow);
                t += 1.0f;
            }
        }

        array[index + 1] = current;
    }

    const float radius = 0.50f * (spine1.y - head.y);
    if (DrawHead)
        DrawCircle(ImVec2(head.x, head.y - radius), radius, color, shadow, glow);
}

void shadow_bar(const ImVec2 &pos1, const ImVec2 &pos2)
{
    ImU32 shadowColor = IM_COL32(0, 0, 0, 255);
    float shadowOffset = 1.0f;
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    drawList->AddRectFilledMultiColor(ImVec2(pos1.x - shadowOffset, pos1.y - shadowOffset),
                                      ImVec2(pos2.x + shadowOffset, pos1.y), shadowColor, shadowColor, shadowColor, shadowColor);
    drawList->AddRectFilledMultiColor(ImVec2(pos1.x - shadowOffset, pos1.y),
                                      ImVec2(pos1.x, pos2.y + shadowOffset), shadowColor, shadowColor, shadowColor, shadowColor);
    drawList->AddRectFilledMultiColor(ImVec2(pos2.x, pos1.y),
                                      ImVec2(pos2.x + shadowOffset, pos2.y + shadowOffset), shadowColor, shadowColor, shadowColor, shadowColor);
    drawList->AddRectFilledMultiColor(ImVec2(pos1.x, pos2.y),
                                      ImVec2(pos2.x + shadowOffset, pos2.y + shadowOffset), shadowColor, shadowColor, shadowColor, shadowColor);
}

ImVec2 CenterText(const char *text, ImFont *font, float tsize, float x, float xw)
{
    auto strd = strdup(text);
    ImVec2 text_size = font->CalcTextSizeA(tsize, FLT_MAX, -1.0f, strd);
    free(strd);
    text_size.x = (float)(int)(text_size.x + 0.99999f);
    return {x + (xw - text_size.x) * 0.5f, text_size.y};
}

void esp::render()
{
    if (!g.b_esp)
        return;
    if (!c_player || !c_player->local)
        return;
    if (!m_have_matrix)
        return;

    std::vector<EspSnap> snaps;
    {
        std::lock_guard<std::mutex> lock(m_snap_mu);
        snaps = m_snap;
    }
    if (snaps.empty())
        return;

    auto draw = ImGui::GetBackgroundDrawList();
    const Matrix w2c = m_cached;

    for (const EspSnap &snap : snaps)
    {
        c_player_controller *player = snap.player;
        if (!player)
            continue;

        int health = snap.health;
        if (health < 0)
            health = 100;
        if (health <= 0)
            continue;

        // Halalium geometry (±1.7 / -0.1) from Unity-thread snapshot
        Vector3 foot = snap.foot;
        Vector3 head = snap.head;
        if (foot == Vector3{} || head == Vector3{})
            continue;

        Vector3 footpos = c_globals->world2screen(w2c, foot);
        Vector3 headpos = c_globals->world2screen(w2c, head);
        float height = fabsf(footpos.y - headpos.y);
        if (footpos.z < 0.01f || headpos.z < 0.01f)
            continue;

        float width = height / 1.9f; // Halalium aspect
        float x = headpos.x - width * 0.5f;
        float y = headpos.y;

        if (g.b_rect)
        {
            // Halalium Lemming box (@0x1e9f58 / corners @0x1ea37c)
            ImU32 col = ImGui::ColorConvertFloat4ToU32(
                ImVec4(g.m_rect[0], g.m_rect[1], g.m_rect[2], g.m_rect[3]));
            const float th = std::clamp(g.f_box_thickness, 0.5f, 5.f);
            const float rounding = (g.i_box_type == 2) ? 3.f : 0.f;
            const ImVec2 p1(x, y);
            const ImVec2 p2(x + width, y + height);

            auto draw_corners = [&](ImU32 c, float t, float inset) {
                const float cf = std::clamp(g.f_corner_size, 0.05f, 0.5f);
                const float bw = (p2.x - p1.x);
                const float bh = (p2.y - p1.y);
                const float cx = std::clamp(bw * cf, 2.f, bw * 0.5f);
                const float cy = std::clamp(bh * cf, 2.f, bh * 0.5f);
                const float x1 = p1.x - inset, y1 = p1.y - inset;
                const float x2 = p2.x + inset, y2 = p2.y + inset;
                draw->AddLine(ImVec2(x1, y1), ImVec2(x1 + cx, y1), c, t);
                draw->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 + cy), c, t);
                draw->AddLine(ImVec2(x2, y1), ImVec2(x2 - cx, y1), c, t);
                draw->AddLine(ImVec2(x2, y1), ImVec2(x2, y1 + cy), c, t);
                draw->AddLine(ImVec2(x1, y2), ImVec2(x1 + cx, y2), c, t);
                draw->AddLine(ImVec2(x1, y2), ImVec2(x1, y2 - cy), c, t);
                draw->AddLine(ImVec2(x2, y2), ImVec2(x2 - cx, y2), c, t);
                draw->AddLine(ImVec2(x2, y2), ImVec2(x2, y2 - cy), c, t);
            };

            if (g.b_box_fill)
            {
                const float fa = std::clamp(g.f_box_fill_alpha, 0.f, 1.f) *
                                 (float)((col >> 24) & 0xff);
                const ImU32 fc = (col & 0x00FFFFFF) | ((ImU32)fa << 24);
                draw->AddRectFilled(p1, p2, fc, rounding);
            }
            if (g.b_box_outline)
            {
                const ImU32 oc = IM_COL32(0, 0, 0, 0xA0);
                const float ot = th + 1.f;
                const float half = th * 0.5f;
                if (g.i_box_type == 1)
                    draw_corners(oc, ot, half);
                else
                    draw->AddRect(ImVec2(p1.x - half, p1.y - half),
                                  ImVec2(p2.x + half, p2.y + half),
                                  oc, rounding, 0, ot);
            }
            if (g.i_box_type == 1)
                draw_corners(col, th, 0.f);
            else
                draw->AddRect(p1, p2, col, rounding, 0, th);
        }

        if (g.b_distance)
        {
            Vector3 cam_pos = m_cam_pos;
            const int distm = (cam_pos == Vector3{})
                                  ? 0
                                  : (int)(Vector3::Distance(cam_pos, foot) + 0.5f);
            char dbuf[24]{};
            std::snprintf(dbuf, sizeof(dbuf), "%dm", distm); // Halalium
            const float fontSize = 14.f;
            ImVec2 text_size = gui::font ? gui::font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, dbuf)
                                        : ImGui::CalcTextSize(dbuf);
            float textX = x + (width * 0.5f) - (text_size.x * 0.5f);
            float textY = y + height + 2.f;
            ImU32 dcol = ImGui::ColorConvertFloat4ToU32(
                ImVec4(g.m_distance[0], g.m_distance[1], g.m_distance[2], g.m_distance[3]));
            if (gui::font)
                text(gui::font, fontSize, ImVec2(textX, textY), dcol, dbuf, false, true);
            else
                draw->AddText(ImVec2(textX, textY), dcol, dbuf);
        }

        if (g.b_health)
        {
            // Halalium: fixed 3px bar at x-6..x-3, red→green gradient
            const float hp = std::clamp(health / 100.f, 0.f, 1.f);
            const ImVec2 bg1(x - 6.f, y);
            const ImVec2 bg2(x - 3.f, y + height);
            draw->AddRectFilled(bg1, bg2, IM_COL32(0, 0, 0, 160));

            const float fy = y + height * (1.f - hp);
            ImVec4 cmin(g.m_health_min[0], g.m_health_min[1], g.m_health_min[2], g.m_health_min[3]);
            ImVec4 cmax(g.m_health[0], g.m_health[1], g.m_health[2], g.m_health[3]);
            ImVec4 c{
                cmin.x + (cmax.x - cmin.x) * hp,
                cmin.y + (cmax.y - cmin.y) * hp,
                cmin.z + (cmax.z - cmin.z) * hp,
                1.f};
            const ImU32 hc = ImGui::ColorConvertFloat4ToU32(c);
            const ImU32 hmin = ImGui::ColorConvertFloat4ToU32(cmin);
            draw->AddRectFilledMultiColor(ImVec2(bg1.x, fy), bg2, hc, hc, hmin, hmin);

            if (g.b_hp_number)
            {
                char healthText[16];
                snprintf(healthText, sizeof(healthText), "%d", health);
                ImFont *pf = gui::pixel ? gui::pixel : ImGui::GetFont();
                if (pf)
                {
                    ImVec2 textSize = pf->CalcTextSizeA(10, FLT_MAX, 0, healthText);
                    float textX = x - 6.f - textSize.x - 2.f;
                    float textY = fy - textSize.y * 0.5f;
                    text(pf, 10, ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), healthText, true, false);
                }
            }
        }
    }
}
