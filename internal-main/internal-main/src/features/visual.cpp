#include "visual.h"
#include <imgui.h>
#include "esp.h"
#include "imgui_internal.h"
#include "src/menu/gui.h"

struct trasers
{
    Vector3 start;
    Vector3 end;
    float time;
};
int i_damage;
static std::vector<trasers> c_tracers;
std::vector<std::array<Vector3, 22>> c_skeletons;

void visual::hits(c_player_controller *local)
{
    if (!target_t.fire)
        return;
    if (!local)
        return;
    auto weapon = c_player->gun_controller;
    if (!weapon) return;
    if (!c_globals->holding_gun())
        return;
    auto hits = weapon->m_pdCharacterHits;
    if (!hits || !hits->entries)
        return;
    auto entries = hits->entries;
    if (!entries->m_Items)
        return;

    for (int i = 0; i < hits->count; ++i)
    {
        auto a = entries->m_Items[i];
        if (a.hashCode < 0)
            continue;
        auto list_data = a.value;
        if (!list_data || !list_data->items || !list_data->items->m_Items)
            continue;
        for (int j = 0; j < list_data->size; ++j)
        {
            auto hit = list_data->items->m_Items[j];
            if (!hit)
                continue;
            i_damage = hit->m_iDamage;
            target_t.damage = hit->m_iDamage;
            if (hit->m_iDamage > 0)
            {
                trasers t;
                t.start = hit->start;
                t.end = hit->end;
                t.time = (float)ImGui::GetTime();
                c_tracers.push_back(t);
            }
        }
    }

    float cur = (float)ImGui::GetTime();
    for (auto it = c_tracers.begin(); it != c_tracers.end();)
    {
        if (cur - it->time > 2.3f)
            it = c_tracers.erase(it);
        else
            ++it;
    }
}

void visual::draw_hits() {
    c_player_controller *local = c_player->local;
    if (!local)
        return;
    if (!g.b_tracer)
        return;

    auto draw = ImGui::GetBackgroundDrawList();
    float cur_time = (float)ImGui::GetTime();
    auto matrix = c_esp->matrix();

    for (size_t i = 0; i < c_tracers.size();)
    {
        auto &t = c_tracers[i];
        float life = cur_time - t.time;
        if (life >= 2.3f)
        {
            c_tracers.erase(c_tracers.begin() + i);
            continue;
        }

        Vector3 s = c_globals->world2screen(matrix, t.start);
        Vector3 e = c_globals->world2screen(matrix, t.end);

        if (s.z <= 0.f || e.z <= 0.f)
        {
            ++i;
            continue;
        }

        float alpha = 1.f - (life / 2.3f);
        if (alpha < 0.f)
            alpha = 0.f;
        if (alpha > 1.f)
            alpha = 1.f;

        color_t base = float4tocolor(g.m_tracer);
        ImU32 col = ImColor(base.r, base.g, base.b, base.a * alpha);

        draw->AddLine(ImVec2(s.x, s.y), ImVec2(e.x, e.y), col, 1.f);
        ++i;
    }
}

void visual::hitmarker() {
    c_player_controller *local = c_player->local;
    if (!local)
        return;

    auto draw = ImGui::GetBackgroundDrawList();
    float cur_time = (float)ImGui::GetTime();
    auto matrix = c_esp->matrix();

    for (size_t i = 0; i < c_tracers.size();)
    {
        auto &t = c_tracers[i];
        float life = cur_time - t.time;
        if (life >= 2.3f)
        {
            c_tracers.erase(c_tracers.begin() + i);
            continue;
        }

        char damageStr[16]{};
        snprintf(damageStr, sizeof(damageStr), "%d", i_damage);

        Vector3 end = c_globals->world2screen(matrix, t.end);

        if (end.z <= 0.f)
        {
            ++i;
            continue;
        }

        float alpha = 1.f - (life / 2.3f);
        if (alpha < 0.f)
            alpha = 0.f;
        if (alpha > 1.f)
            alpha = 1.f;

        color_t base = float4tocolor(g.m_marker);
        ImU32 color = ImColor(base.r, base.g, base.b, base.a * alpha);

        if (g.b_marker)
        {
            draw->AddLine(ImVec2(end.x - 8, end.y - 8), ImVec2(end.x - 2.f, end.y - 2.f), color, 1.8f);
            draw->AddLine(ImVec2(end.x + 8, end.y - 8), ImVec2(end.x + 2.f, end.y - 2.f), color, 1.8f);
            draw->AddLine(ImVec2(end.x - 8, end.y + 8), ImVec2(end.x - 2.f, end.y + 2.f), color, 1.8f);
            draw->AddLine(ImVec2(end.x + 8, end.y + 8), ImVec2(end.x + 2.f, end.y + 2.f), color, 1.8f);
        }
        if (g.b_dmarker)
        {
            draw->AddText(gui::font, 15, ImVec2(end.x, end.y - 40), color, damageStr);
        }

        ++i;
    }
}

void visual::remove(c_player_controller *local) {
    if (!g.b_scope) return;
    if (!local)
        return;

    if (c_player->gun_parameters) {
        if (c_globals->holding_gun()) {

            if (c_globals->is_scoped()) {
                auto controls = c_player->controls;
                if (!controls)
                    return;

                c_camera cam;
                auto camera = (c_camera *)cam.get_main();
                if (!camera)
                    return;

                camera->set_fov(59.9);

                if (!g.b_scope)
                    return;

                // PlayerControls.HUDView @ 0xB8; AimView @ HUD+0x30; _sniperSight @ Aim+0x50
                auto hudView = *(void **)((uintptr_t)controls + oxorany(0xb8));
                if (!hudView)
                    return;

                auto aimView = *(c_component **)((uintptr_t)hudView + oxorany(0x30));
                if (!aimView)
                    return;

                auto sniper = *(c_component **)((uintptr_t)aimView + oxorany(0x50));
                if (!sniper)
                    return;

                auto game_object = sniper->get_game_object();
                if (!game_object)
                    return;

                c_methods->set_active(game_object, false);
            }
        }
    }
}

void visual::third_view(c_transform *player_transform) {
    if (g.b_third) {
        if (player_transform) {
            if (c_player->weapon_parameters && c_globals->holding_gun()) {
                static Vector3 smoothed_cam_pos(0, 0, 0);

                c_component component{};
                c_transform *transform{};
                c_camera camera{};

                Vector3 forward{};
                Vector3 location{};
                Vector3 camera_position{};
                Vector3 target_pos{};

                auto get_main = camera.get_main();
                if (get_main) {
                    transform = component.get_transform(get_main);
                    if (transform) {

                        forward = transform->get_forward();
                        location = player_transform->get_position();

                        camera_position = location + Vector3(0.f, 1.50f, 0.f) + forward * -g.mom;

                        Vector3 start = location + Vector3(0.f, 1.50f, 0.f);
                        Vector3 end = camera_position;

                        ray_t ray{};
                        ray.m_vecOrigin = start;
                        ray.m_vecDirection = (end - start).nnormalized();

                        raycast_hit_t hit{};
                        float max_back = (end - start).mmagnitude();

                        if (!c_methods->sphere_cast(ray, 0.2f, &hit, max_back, 16384))
                            target_pos = camera_position;
                        else
                            target_pos = hit.point;

                        smoothed_cam_pos = ImLerp(smoothed_cam_pos, target_pos, 0.688f);

                        transform->set_position(smoothed_cam_pos);
                    }
                }
            }
        }
    }
}

