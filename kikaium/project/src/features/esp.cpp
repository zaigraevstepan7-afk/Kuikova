#include "esp.h"
#include <imgui.h>
#include <map>
#include <algorithm>
#include <cstdio>
#include <src/menu/gui.h>
#include "sdk/OffsetsBridge.h"
#include "includes/halalium_chains.h"

Matrix esp::matrix()
{
    // Prefer matrix snapshotted on Unity thread; avoid live Unity calls from EGL.
    if (m_have_matrix)
        return m_cached;
    return Matrix{};
}

static bool matrix_nonzero(const Matrix &m)
{
    return m.m00 != 0.f || m.m11 != 0.f || m.m22 != 0.f || m.m33 != 0.f;
}

static bool read_matrix_field(void *obj, uintptr_t off, Matrix *out)
{
    if (!obj || !out || !c_globals || !c_globals->is_allocated(obj))
        return false;
    if (!c_globals->is_allocated((void *)((uintptr_t)obj + off)))
        return false;
    Matrix m = *(Matrix *)((uintptr_t)obj + off);
    if (!matrix_nonzero(m))
        return false;
    *out = m;
    return true;
}

// Halalium-proven: PlayerMainCamera+0x28 → +0x30 → Unity Camera*, then Camera.matrix@0xF0
static bool read_halalium_camera_matrix(void *player_main_camera, Matrix *out)
{
    if (!player_main_camera || !out)
        return false;

    void *nested = *(void **)((uintptr_t)player_main_camera + Offsets::PlayerMainCamera::nested);
    if (!nested || !c_globals->is_allocated(nested))
        return false;
    void *ucam = *(void **)((uintptr_t)nested + Offsets::PlayerMainCamera::unity_camera);
    if (!ucam || !c_globals->is_allocated(ucam))
        return false;

    // Profile: Camera.matrix @0xF0 on the resolved camera object
    if (read_matrix_field(ucam, Offsets::Camera::matrix, out))
        return true;
    if (read_matrix_field(ucam, Offsets::Camera::matrix_alt, out))
        return true;

    // UnityEngine.Object m_CachedPtr @+0x10 → native matrix @0xF0
    void *native = *(void **)((uintptr_t)ucam + 0x10);
    if (native && c_globals->is_allocated(native))
    {
        if (read_matrix_field(native, Offsets::Camera::matrix, out))
            return true;
        if (read_matrix_field(native, Offsets::Camera::matrix_alt, out))
            return true;
    }

    // Engine API on Halalium-resolved Camera* (last Halalium-aligned option)
    if (c_fn && c_fn->get_w2c_injected)
    {
        Matrix m{};
        c_fn->get_w2c_injected(ucam, &m);
        if (matrix_nonzero(m))
        {
            *out = m;
            return true;
        }
    }
    return false;
}

// Melodium/community nest — NOT Halalium RE; kept only as emergency fallback
static bool read_community_camera_matrix(void *player_main_camera, Matrix *out)
{
    if (!player_main_camera || !out || !c_globals || !c_globals->is_allocated(player_main_camera))
        return false;

    void *ptr = *(void **)((uintptr_t)player_main_camera + Offsets::Camera::community_a);
    if (!ptr || !c_globals->is_allocated(ptr))
        return false;
    void *cashed = *(void **)((uintptr_t)ptr + Offsets::Camera::community_b);
    if (!cashed || !c_globals->is_allocated(cashed))
        return false;

    if (read_matrix_field(cashed, Offsets::Camera::matrix, out))
        return true;
    return read_matrix_field(cashed, Offsets::Camera::matrix_alt, out);
}

void esp::cache_matrix()
{
    Matrix mat{};
    bool ok = false;

    void *pmc = nullptr;
    if (c_player && c_player->local)
    {
        pmc = c_player->local->m_pMainCamera;
        if (!pmc)
            pmc = hchain::main_camera(c_player->local);
    }

    // 1) Halalium FOV nest + profile matrix@0xF0
    if (pmc && read_halalium_camera_matrix(pmc, &mat))
        ok = true;

    // 2) Camera.main via same Halalium field reads if get_main returns Camera*
    if (!ok && c_fn && c_fn->camera_get_main)
    {
        void *cam = c_fn->camera_get_main();
        if (cam && c_globals->is_allocated(cam))
        {
            if (read_matrix_field(cam, Offsets::Camera::matrix, &mat) ||
                read_matrix_field(cam, Offsets::Camera::matrix_alt, &mat))
            {
                ok = true;
            }
            else if (c_fn->get_w2c_injected)
            {
                c_fn->get_w2c_injected(cam, &mat);
                if (matrix_nonzero(mat))
                    ok = true;
            }
        }
    }

    // 3) Melodium nest fallback only
    if (!ok && pmc && read_community_camera_matrix(pmc, &mat))
        ok = true;

    m_cached = mat;
    m_have_matrix = ok;
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
    // Entities come from PC.Update; GameController TypeInfo is not required to draw.
    if (!c_player || !c_player->local)
        return;
    if (!m_have_matrix)
        return;
    if (c_player->entity.empty())
        return;

    {
        c_player_controller *player{};
        c_photon_player *photon{};
        c_transform *transform{};
        int health{};
        monoString *name{};
        float x, y;
        auto draw = ImGui::GetBackgroundDrawList();
        const Matrix w2c = m_cached;

        static std::map<void*, float> fadeT;
        static std::map<void*, bool> wasAlive;
        const float fadeDuration = 1.7f;
        const float fadeStep = 1.0f / (fadeDuration * 60.0f);

        auto easeInOut = [&](float t)
        {
            return 0.5f - 0.5f * cosf(3.1415926535f * t);
        };

        auto easeOut = [&](float t)
        {
            return 0.5f + 0.5f * cosf(3.1415926535f * t);
        };

        for (int i{}; i < (int)c_player->entity.size(); i++)
        {
            player = c_player->entity[i];
            if (!player || !c_globals->is_allocated(player))
                continue;
            if (!c_globals->is_enemy(c_player->local, player))
                continue;

            photon = player->m_pPhoton;
            transform = player->m_pTransform;
            if (!photon || !transform)
                continue;
            if (!c_globals->is_allocated(photon) || !c_globals->is_allocated(transform))
                continue;

            health = photon->get_health();
            // Property miss (-1) must not kill ESP — still draw as full HP
            if (health < 0)
                health = 100;

            bool alive = health > 0;

            if (!fadeT.count(player))
            {
                fadeT[player] = 1.0f; // show immediately — no multi-frame fade-in gate
                wasAlive[player] = alive;
            }

            float &t = fadeT[player];

            if (alive)
            {
                wasAlive[player] = true;
                if (t < 1.0f)
                {
                    t += fadeStep;
                    if (t > 1.0f) t = 1.0f;
                }
            }
            else
            {
                if (t > 0.0f)
                {
                    t -= fadeStep;
                    if (t < 0.0f) t = 0.0f;
                }
            }

            if (t <= 0.01f)
                continue;

            float alpha = alive ? easeInOut(t) : easeOut(t);

            auto ApplyAlpha = [&](ImU32 col)
            {
                ImVec4 c = ImGui::ColorConvertU32ToFloat4(col);
                c.w *= alpha;
                return ImGui::ColorConvertFloat4ToU32(c);
            };

            Vector3 foot = transform->get_position();
            if (foot == Vector3{})
                continue;

            Vector3 head{foot.x, foot.y + 1.65f, foot.z};
            Vector3 footpos = c_globals->world2screen(w2c, foot);
            Vector3 headpos = c_globals->world2screen(w2c, head);
            float height = fabsf(footpos.y - headpos.y);

            if (footpos.z < 0.01f || headpos.z < 0.01f)
                continue;

            float width = height / 1.8f;
            x = headpos.x - width * 0.5f;
            y = headpos.y;

            if (g.b_skeleton)
                DrawSkeleton({g.m_skeleton[0], g.m_skeleton[1], g.m_skeleton[2], g.m_skeleton[3] * alpha}, false, false, false, w2c, player);

            if (g.b_rect)
            {
                ImVec4 color{g.m_rect[0], g.m_rect[1], g.m_rect[2], g.m_rect[3]};
                ImU32 col_box = ApplyAlpha(ImGui::ColorConvertFloat4ToU32(color));
                ImU32 col_shadow = ApplyAlpha(IM_COL32(0, 0, 0, 255));

                float x1 = x;
                float y1 = y;
                float x2 = x + width;
                float y2 = y + height;

                if (g.i_box_type == 1)
                {
                    // Halalium "Box Type" = Corner
                    const float cs = std::clamp(g.f_corner_size, 2.f, 40.f);
                    auto corner = [&](float ax, float ay, float dx, float dy) {
                        draw->AddLine(ImVec2(ax, ay), ImVec2(ax + dx, ay), col_shadow, 3.f);
                        draw->AddLine(ImVec2(ax, ay), ImVec2(ax, ay + dy), col_shadow, 3.f);
                        draw->AddLine(ImVec2(ax, ay), ImVec2(ax + dx, ay), col_box, 1.5f);
                        draw->AddLine(ImVec2(ax, ay), ImVec2(ax, ay + dy), col_box, 1.5f);
                    };
                    corner(x1, y1, cs, cs);
                    corner(x2, y1, -cs, cs);
                    corner(x1, y2, cs, -cs);
                    corner(x2, y2, -cs, -cs);
                }
                else
                {
                    draw->AddRectFilledMultiColor({x1 - 1, y1 - 1}, {x2 + 1, y1}, col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor({x1 - 1, y2}, {x2 + 1, y2 + 1}, col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor({x1 - 1, y1}, {x1, y2}, col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor({x2, y1}, {x2 + 1, y2}, col_shadow, col_shadow, col_shadow, col_shadow);

                    draw->AddRectFilledMultiColor(ImVec2(x1, y1 + 1), ImVec2(x2, y1 + 2), col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor(ImVec2(x1, y2 - 2), ImVec2(x2, y2 - 1), col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor(ImVec2(x1 + 1, y1), ImVec2(x1 + 2, y2), col_shadow, col_shadow, col_shadow, col_shadow);
                    draw->AddRectFilledMultiColor(ImVec2(x2 - 2, y1), ImVec2(x2 - 1, y2), col_shadow, col_shadow, col_shadow, col_shadow);

                    draw->AddRectFilledMultiColor({x1, y1}, {x2, y1 + 1}, col_box, col_box, col_box, col_box);
                    draw->AddRectFilledMultiColor({x1, y2 - 1}, {x2, y2}, col_box, col_box, col_box, col_box);
                    draw->AddRectFilledMultiColor({x1, y1}, {x1 + 1, y2}, col_box, col_box, col_box, col_box);
                    draw->AddRectFilledMultiColor({x2 - 1, y1}, {x2, y2}, col_box, col_box, col_box, col_box);
                }
            }

            if (g.b_distance)
            {
                Vector3 cam_pos{};
                if (c_player->local && c_player->local->m_pMainCameraHolder)
                    cam_pos = c_player->local->m_pMainCameraHolder->get_position();
                const float distm = (cam_pos == Vector3{}) ? 0.f : Vector3::Distance(cam_pos, foot);
                char dbuf[24]{};
                std::snprintf(dbuf, sizeof(dbuf), "%.0fm", distm);
                const float fontSize = 14.f;
                ImVec2 text_size = gui::font ? gui::font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, dbuf)
                                            : ImGui::CalcTextSize(dbuf);
                float textX = x + (width * 0.5f) - (text_size.x * 0.5f);
                float textY = y + height + 2.f;
                ImU32 dcol = ApplyAlpha(ImGui::ColorConvertFloat4ToU32(
                    ImVec4(g.m_distance[0], g.m_distance[1], g.m_distance[2], g.m_distance[3])));
                if (gui::font)
                    text(gui::font, fontSize, ImVec2(textX, textY), dcol, dbuf, false, true);
                else
                    draw->AddText(ImVec2(textX, textY), dcol, dbuf);
            }

            if (g.b_name)
            {
                const float fontSize = 15.0f;
                auto player_name = photon->m_nameField;
                if (player_name)
                {
                    auto nameStr = player_name->toUTF8();
                    ImFont *nf = gui::font ? gui::font : ImGui::GetFont();
                    if (nf && !nameStr.empty())
                    {
                        ImVec2 text_size = nf->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, nameStr.c_str());
                        float textX = x + (width * 0.5f) - (text_size.x * 0.5f);
                        float textY = y - text_size.y - 2.0f;
                        text(nf, fontSize, ImVec2(textX, textY), ApplyAlpha(ImColor(255, 255, 255, 255)), nameStr.c_str(), false, true);
                    }
                }
            }

            if (g.b_ammo)
            {
                const float optimalSize = width * 0.0625f + 8.0f;
                auto healthWidth = round(optimalSize * 0.2f);
                auto padding = optimalSize * 0.350f;
                ImVec4 color{g.m_ammo[0], g.m_ammo[1], g.m_ammo[2], g.m_ammo[3]};
                auto weaponry = player->m_pWeaponry;
                if (weaponry && c_globals->is_allocated(weaponry) && weaponry->m_pCurrentWeapon)
                {
                    auto weapon = (c_gun_controller *)weaponry->m_pCurrentWeapon;
                    auto *params = weapon->m_pParameters;
                    if (params && params->m_id >= 11 && params->m_id <= 65)
                    {
                        auto ammo = weapon->m_iAmmoSafe.get();
                        const short max_ammo = 30;
                        float ammo_progress = std::clamp((float)ammo / (float)max_ammo, 0.0f, 1.0f);
                        float barWidth = width;
                        float barHeight = healthWidth;
                        float barX = x;
                        float barY = y + height + padding;
                        ImVec2 barStart = ImVec2(barX, barY);
                        ImVec2 barEnd = ImVec2(barX + barWidth, barY + barHeight);
                        ImVec2 fillStart = ImVec2(barX, barY);
                        ImVec2 fillEnd = ImVec2(barX + barWidth * ammo_progress, barY + barHeight);
                        shadow_bar(barStart, barEnd);
                        draw->AddRectFilled(barStart, barEnd, ApplyAlpha(ImColor(0, 0, 0, 115)));
                        ImU32 colL = ApplyAlpha(ImGui::ColorConvertFloat4ToU32(color));
                        draw->AddRectFilledMultiColor(fillStart, fillEnd, colL, colL, colL, colL);
                    }
                }
            }

            if (g.b_eweapon)
            {
                auto weaponry = player->m_pWeaponry;
                if (!weaponry || !c_globals->is_allocated(weaponry))
                    continue;
                auto weapon = (c_gun_controller*)weaponry->m_pCurrentWeapon;
                if (!weapon)
                    continue;
                auto *params = weapon->m_pParameters;
                if (!params)
                    continue;
                auto id = params->m_id;

                const char *weaponName;
                switch (id)
                {
                case 11: weaponName = "G22"; break;
                case 12: weaponName = "USP"; break;
                case 13: weaponName = "P350"; break;
                case 15: weaponName = "Deagle"; break;
                case 16: weaponName = "TEC9"; break;
                case 17: weaponName = "Five-Seven"; break;
                case 18: weaponName = "Berretas"; break;
                case 32: weaponName = "UMP45"; break;
                case 33: weaponName = "Akimbo Uzi"; break;
                case 34: weaponName = "MP7"; break;
                case 35: weaponName = "P90"; break;
                case 36: weaponName = "MP5"; break;
                case 37: weaponName = "MAC10"; break;
                case 42: weaponName = "Val"; break;
                case 43: weaponName = "M4A1"; break;
                case 44: weaponName = "AKR"; break;
                case 45: weaponName = "AKR12"; break;
                case 46: weaponName = "M4"; break;
                case 47: weaponName = "M16"; break;
                case 48: weaponName = "Famas"; break;
                case 49: weaponName = "Fnfal"; break;
                case 51: weaponName = "AWM"; break;
                case 52: weaponName = "M40"; break;
                case 53: weaponName = "M110"; break;
                case 54: weaponName = "Mallard"; break;
                case 62: weaponName = "SM1014"; break;
                case 63: weaponName = "FabM"; break;
                case 64: weaponName = "M60"; break;
                case 65: weaponName = "Spas"; break;
                case 70: weaponName = "Knife"; break;
                case 71: weaponName = "Bayonet"; break;
                case 72: weaponName = "karambit"; break;
                case 73: weaponName = "Jkommando"; break;
                case 75: weaponName = "Butterfly"; break;
                case 77: weaponName = "Flip"; break;
                case 78: weaponName = "Kunai"; break;
                case 79: weaponName = "Scorpion"; break;
                case 80: weaponName = "Tanto"; break;
                case 81: weaponName = "Dagger"; break;
                case 82: weaponName = "Kukri"; break;
                case 83: weaponName = "Stilet"; break;
                case 85: weaponName = "Mantis"; break;
                case 86: weaponName = "Fang"; break;
                case 88: weaponName = "Sting"; break;
                case 89: weaponName = "Hands"; break;
                case 91: weaponName = "HE"; break;
                case 93: weaponName = "Flash"; break;
                case 92: weaponName = "Smoke"; break;
                case 94: weaponName = "Molotov"; break;
                case 95: weaponName = "Incendiary"; break;
                case 100: weaponName = "Bomb"; break;
                default: weaponName = "Unknown"; break;
                }

                const float optimalSize = width * 0.0625f + 8.0f;
                const float font_size = 15.0f;
                const float fixed_text_height = font_size;
                ImFont *wf = gui::font ? gui::font : ImGui::GetFont();
                if (!wf)
                    continue;

                ImU32 colTxt = ApplyAlpha(ImColor(255, 255, 255, 255));

                if (!g.b_ammo)
                {
                    text(wf, font_size,
                        ImVec2(
                            CenterText(weaponName, wf, font_size, x, width).x,
                            y + height + optimalSize * 1.4f - fixed_text_height),
                        colTxt, weaponName, false, true);
                }
                else if (id >= 11 && id <= 65)
                {
                    text(wf, font_size,
                        ImVec2(
                            CenterText(weaponName, wf, font_size, x, width).x,
                            y + height + optimalSize * 2.3f - fixed_text_height),
                        colTxt, weaponName, false, true);
                }
                else if (id > 65 && id <= 100)
                {
                    text(wf, font_size,
                        ImVec2(
                            CenterText(weaponName, wf, font_size, x, width).x,
                            y + height + optimalSize * 1.4f - fixed_text_height),
                        colTxt, weaponName, false, true);
                }
            }

            if (g.b_health)
            {
                ImVec4 color{g.m_health[0], g.m_health[1], g.m_health[2], g.m_health[3]};
                float healthOffset = 0.350f;
                const float optimalSize = width * 0.0625f + 8.0f;
                float healthWidth = roundf(optimalSize * 0.2f);
                float padding = optimalSize * healthOffset;
                float healthPos = health / 100.0f;
                if (healthPos < 0.0f) healthPos = 0.0f;
                if (healthPos > 1.0f) healthPos = 1.0f;

                ImVec2 pos1(x - healthWidth - padding, y);
                ImVec2 pos2(x - padding, y + height);

                draw->AddRectFilled(pos1, pos2, ApplyAlpha(IM_COL32(0, 0, 0, 128)));

                shadow_bar(pos1, pos2);

                draw->AddRectFilled({pos1.x, pos1.y}, {pos2.x, pos2.y}, ApplyAlpha(ImColor(0, 0, 0, 120)));
                draw->AddRectFilled({x - healthWidth - padding, y + roundf(height * (1.0f - healthPos))},
                                    {x - padding, y + height}, ApplyAlpha(ImGui::ColorConvertFloat4ToU32(color)));

                if (health < 100)
                {
                    char healthText[16];
                    snprintf(healthText, sizeof(healthText), "%d", health);
                    ImFont *pf = gui::pixel ? gui::pixel : ImGui::GetFont();
                    if (pf)
                    {
                        ImVec2 textSize = pf->CalcTextSizeA(10, FLT_MAX, 0, healthText);
                        float textX = x - healthWidth - padding + (healthWidth - textSize.x) * 0.5f;
                        float textY = y + height * (1.0f - healthPos) - textSize.y + 2.0f;
                        text(pf, 10, ImVec2(textX, textY), ApplyAlpha(ImColor(255, 255, 255, 255)), healthText, true, false);
                    }
                }
            }
        }
    }
}
