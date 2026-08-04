#include "gui.h"
#include <globals.hpp>
#include "c_user_interface.hpp"

float alpha = 0.0f;
inline C_UserInterface ui;
namespace gui
{
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
        float startThickness = 4.0f;
        ImU32 startColor = IM_COL32(255, 255, 255, 150);
        ImU32 endColor = IM_COL32(220, 220, 220, 150);

        float gap = 40.0f;

        DrawGradientLine(drawList, ImVec2(center.x, center.y - lineLength), ImVec2(center.x, center.y - gap), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x, center.y + lineLength), ImVec2(center.x, center.y + gap), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x - lineLength, center.y), ImVec2(center.x - gap, center.y), startThickness, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x + lineLength, center.y), ImVec2(center.x + gap, center.y), startThickness, startColor, endColor);
    }

    // Kikaium watermark — distinct from Halalium pill layout.
    void DrawWatermark()
    {
        const char *brand = oxorany("Kikaium");
        const char *line = oxorany("Kikaium | private | 0392");

        ImGui::SetNextWindowPos(ImVec2(16.f, 16.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin(oxorany("##kik_wm"), nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_AlwaysAutoResize);

        ImVec2 brand_sz = ImGui::CalcTextSize(brand);
        ImVec2 line_sz = ImGui::CalcTextSize(line);
        float width = (brand_sz.x > line_sz.x ? brand_sz.x : line_sz.x) + 28.f;
        float height = brand_sz.y + line_sz.y + 20.f;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 rmin = p;
        ImVec2 rmax = ImVec2(p.x + width, p.y + height);

        // Flat bar + teal underline (not Halalium dark pill + left accent).
        dl->AddRectFilled(rmin, rmax, IM_COL32(12, 16, 18, 200), 0.0f);
        dl->AddRectFilled(ImVec2(rmin.x, rmax.y - 2.f), rmax, IM_COL32(45, 180, 160, 255), 0.0f);

        ImVec2 brand_pos = ImVec2(p.x + 14.f, p.y + 5.f);
        ImVec2 line_pos = ImVec2(p.x + 14.f, p.y + 5.f + brand_sz.y + 2.f);
        dl->AddText(brand_pos, IM_COL32(240, 244, 242, 255), brand);
        dl->AddText(line_pos, IM_COL32(120, 160, 150, 255), line);

        ImGui::Dummy(ImVec2(width, height));
        ImGui::SetCursorScreenPos(rmin);
        if (ImGui::InvisibleButton(oxorany("##kik_wm_click"), ImVec2(width, height)))
            open = !open;

        ImGui::End();
    }

    void other()
    {
        const ImVec2 pos{}, size{(float)c_egl->width, (float)c_egl->heigth};

        ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::Begin(oxorany("1488"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

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
            alpha += 0.025f;
        else if (!open && alpha > 0.025f)
            alpha -= 0.025f;
    }
}
