#include "esp.h"
#include <imgui.h>
#include <map>
#include <algorithm>
#include <cstdio>
#include <src/menu/gui.h>
#include "sdk/OffsetsBridge.h"
#include "includes/halalium_mem.h"

Matrix esp::matrix()
{
    if (m_have_matrix)
        return m_cached;
    return Matrix{};
}

static bool matrix_nonzero(const Matrix &m)
{
    return m.m00 != 0.f || m.m11 != 0.f || m.m22 != 0.f || m.m33 != 0.f;
}

// Exact Halalium chain via hmem LDR (same as Halalium in-process reads):
// Player+0xE8 → +0x28 → +0x30 → Unity Camera → matrix@0xF0
void esp::cache_matrix()
{
    Matrix mat{};
    bool ok = false;

    if (c_player && c_player->local)
    {
        void *player = c_player->local;
        void *pmc = hmem::field_ptr(player, Offsets::Player::main_camera);
        if (pmc)
        {
            void *nested = hmem::field_ptr(pmc, Offsets::PlayerMainCamera::nested);
            if (nested)
            {
                void *ucam = hmem::field_ptr(nested, Offsets::PlayerMainCamera::unity_camera);
                if (ucam)
                {
                    if (hmem::read(reinterpret_cast<uintptr_t>(ucam) + Offsets::Camera::matrix, mat) &&
                        matrix_nonzero(mat))
                        ok = true;
                }
            }
        }
    }

    m_cached = mat;
    m_have_matrix = ok;
    dbg_matrix.store(ok ? 1 : 0, std::memory_order_relaxed);
    dbg_local.store((c_player && c_player->local) ? 1 : 0, std::memory_order_relaxed);
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
    for (int i = 0; i < (int)c_player->entity.size(); ++i)
    {
        c_player_controller *player = c_player->entity[i];
        if (!player || !c_globals->is_allocated(player))
            continue;
        if (!c_globals->is_enemy(c_player->local, player))
            continue;
        ++n_enemies;

        if (!g.b_esp)
            continue;

        c_photon_player *photon = player->m_pPhoton;
        c_transform *transform = player->m_pTransform;
        if (!photon || !transform)
            continue;

        EspSnap s{};
        s.player = player;
        s.foot = transform->get_position(); // Unity thread — OK
        if (s.foot == Vector3{})
            continue;

        // Halalium ESP geometry: base± (head +1.7, foot -0.1) — not biped/Melodium +1.65
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
    // Top strip — verify memory reads even when ESP boxes fail
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    if (!dl || !c_egl)
        return;

    const float w = (float)c_egl->width;
    const float bar_h = 22.f;
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(w, bar_h), IM_COL32(0, 0, 0, 180));

    char buf[192]{};
    std::snprintf(buf, sizeof(buf),
                  "xxx | Halalium/Lemming | local:%s | matrix:%s | players:%d | enemies:%d | snap:%d",
                  dbg_local.load(std::memory_order_relaxed) ? "OK" : "NO",
                  dbg_matrix.load(std::memory_order_relaxed) ? "OK" : "NO",
                  dbg_players.load(std::memory_order_relaxed),
                  dbg_enemies.load(std::memory_order_relaxed),
                  dbg_snap.load(std::memory_order_relaxed));

    ImFont *font = gui::font ? gui::font : ImGui::GetFont();
    const float fs = 14.f;
    if (font)
        dl->AddText(font, fs, ImVec2(8.f, 3.f), IM_COL32(255, 200, 80, 255), buf);
    else
        dl->AddText(ImVec2(8.f, 3.f), IM_COL32(255, 200, 80, 255), buf);
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
