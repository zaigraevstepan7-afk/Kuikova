#include "gui.h"
#include <globals.hpp>
#include "c_user_interface.hpp"
#include "includes/kikaium_touch.h"
#include <cmath>

float alpha = 0.0f;
inline C_UserInterface ui;
namespace gui
{
    __attribute__((used, retain)) static const char k_kikaium_contract[] =
        "kikaium_contract:wm_click+egl+vmt+esp";

    void DrawGradientLine(ImDrawList *drawList, ImVec2 start, ImVec2 end, float startThickness, ImU32 startColor, ImU32 endColor, int segments = 20)
    {
        for (int i = 0; i < segments; i++)
        {
            float t0 = (float)i / segments;
            float t1 = (float)(i + 1) / segments;

            ImVec2 p0 = ImVec2(
                start.x + (end.x - start.x) * t0,
                start.y + (end.y - start.y) * t0);

            ImVec2 p1 = ImVec2(
                start.x + (end.x - start.x) * t1,
                start.y + (end.y - start.y) * t1);

            float thickness = startThickness * (1.0f - t0 * 0.9f);

            ImU32 color = IM_COL32(
                (int)((startColor >> 0 & 0xFF) * (1.0f - t0) + (endColor >> 0 & 0xFF) * t0),
                (int)((startColor >> 8 & 0xFF) * (1.0f - t0) + (endColor >> 8 & 0xFF) * t0),
                (int)((startColor >> 16 & 0xFF) * (1.0f - t0) + (endColor >> 16 & 0xFF) * t0),
                (int)((startColor >> 24 & 0xFF) * (1.0f - t0) + (endColor >> 24 & 0xFF) * t0));

            drawList->AddLine(p0, p1, color, thickness);
        }
    }

    void cross()
    {
        ImVec2 center(c_egl->width / 2.0f, c_egl->heigth / 2.0f);
        ImDrawList *drawList = ImGui::GetForegroundDrawList();

        float originalLength = c_egl->heigth / 4.0f;
        float lineLength = originalLength / 2.f;
        float startThickness = 3.0f;
        ImU32 startColor = IM_COL32(236, 164, 72, 160);
        ImU32 endColor = IM_COL32(236, 164, 72, 20);
        float gap = 40.0f;

        DrawGradientLine(drawList, ImVec2(center.x, center.y - lineLength), ImVec2(center.x, center.y - gap), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x, center.y + lineLength), ImVec2(center.x, center.y + gap), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x - lineLength, center.y), ImVec2(center.x - gap, center.y), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x + lineLength, center.y), ImVec2(center.x + gap, center.y), startThickness, startColor, endColor);
    }

    void DrawWatermark()
    {
        if (!g.b_watermark)
            return;
        (void)k_kikaium_contract;
        const char *brand = oxorany("Kikaium");
        const char *line = open ? oxorany("tap to close") : oxorany("private  0.39.2");

        // Soft breathing pulse on accent edge
        const float t = (float)ImGui::GetTime();
        const int pulse_a = 180 + (int)(40.f * sinf(t * 2.2f));

        ImGui::SetNextWindowPos(ImVec2(16.f, 16.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        // Do NOT focus when menu is open - steals tab clicks
        if (!open)
            ImGui::SetNextWindowFocus();

        ImGui::Begin(oxorany("##kik_wm"), nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoNav);

        ImVec2 brand_sz = ImGui::CalcTextSize(brand);
        ImVec2 line_sz = ImGui::CalcTextSize(line);
        float width = (brand_sz.x > line_sz.x ? brand_sz.x : line_sz.x) + 44.f;
        float height = brand_sz.y + line_sz.y + 26.f;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 rmin = p;
        ImVec2 rmax = ImVec2(p.x + width, p.y + height);

        // Layered glass plate
        dl->AddRectFilled(rmin, rmax, IM_COL32(10, 11, 13, 220), 8.f);
        dl->AddRectFilledMultiColor(rmin, rmax,
                                    IM_COL32(28, 26, 22, 40), IM_COL32(18, 18, 20, 10),
                                    IM_COL32(18, 18, 20, 10), IM_COL32(28, 26, 22, 40));
        dl->AddRect(rmin, rmax, IM_COL32(60, 56, 48, 200), 8.f, 0, 1.0f);
        // Copper left spine
        dl->AddRectFilled(ImVec2(rmin.x, rmin.y + 6.f), ImVec2(rmin.x + 3.f, rmax.y - 6.f),
                          IM_COL32(236, 164, 72, pulse_a), 2.f);
        // Bottom accent hairline
        dl->AddRectFilled(ImVec2(rmin.x + 10.f, rmax.y - 2.f), ImVec2(rmax.x - 10.f, rmax.y),
                          IM_COL32(236, 164, 72, 200), 1.f);

        ImVec2 brand_pos = ImVec2(p.x + 18.f, p.y + 8.f);
        ImVec2 line_pos = ImVec2(p.x + 18.f, p.y + 8.f + brand_sz.y + 3.f);
        dl->AddText(brand_pos, IM_COL32(248, 242, 232, 255), brand);
        dl->AddText(line_pos, IM_COL32(190, 145, 90, 255), line);

        ImGui::Dummy(ImVec2(width, height));
        ImGui::SetCursorScreenPos(rmin);
        // Single path: InvisibleButton only (no press_edge / raw_tap double-fire)
        bool clicked = ImGui::InvisibleButton(oxorany("##wm_click"), ImVec2(width, height));
        // Fallback if ImGui miss-hits but InputConsumer saw the tap in-rect
        if (!clicked && kik_input::consume_tap_in_rect(rmin.x, rmin.y, rmax.x, rmax.y))
            clicked = true;

        if (clicked)
        {
            open = !open;
            // Prevent the same tap from clicking checkboxes under the menu
            g_menu_input_lock.store(8, std::memory_order_release);
        }

        ImGui::End();
    }

    void other()
    {
        const ImVec2 pos{}, size{(float)c_egl->width, (float)c_egl->heigth};

        ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::Begin(oxorany("##kik_overlay"), nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);

        if (g.b_scope && c_player->local && c_player->weapon_parameters && c_globals->is_scoped())
            cross();

        ImGui::End();
        DrawWatermark();
    }

    void render()
    {
        other();
        ui.render();

        if (open && alpha < 1.f)
            alpha += 0.06f;
        else if (!open && alpha > 0.02f)
            alpha -= 0.06f;
        if (alpha > 1.f)
            alpha = 1.f;
        if (alpha < 0.f)
            alpha = 0.f;
    }
}
