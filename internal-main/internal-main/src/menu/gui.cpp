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

    void DrawWatermark()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        const char *text = ("t.me/melrele 0.36.1");
        ImVec2 pos = ImVec2(70, 30);
        ImVec2 padding = ImVec2(12, 6);
        ImVec2 text_size = ImGui::CalcTextSize(text);
        ImVec2 rect_min = ImVec2(pos.x - padding.x, pos.y - padding.y);
        ImVec2 rect_max = ImVec2(pos.x + text_size.x + padding.x, pos.y + text_size.y + padding.y);
        float rounding = 8.0f;

        draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(25, 25, 25, 150), rounding);
        draw_list->AddRect(rect_min, rect_max, IM_COL32(70, 70, 70, 150), rounding, 0, 1.5f);

        draw_list->AddText(ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 150), text);
        draw_list->AddText(pos, IM_COL32(220, 220, 220, 250), text);
    }

    void other()
    {
        const ImVec2 pos{}, size{(float)c_egl->width, (float)c_egl->heigth};

        ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::Begin(oxorany("1488"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar); // ImGuiWindowFlags_NoInputs

        auto pDrawList = ImGui::GetWindowDrawList();

        DrawWatermark();

         if (g.b_scope && c_player->local && c_player->weapon_parameters && c_globals->is_scoped())
             cross();

        ImGui::End();
    }
    void render()
    {
        other();
        ui.render();
        ImGui::GetStyle().WindowMinSize = ImVec2(0, 0);
        ImGui::SetNextWindowPos(ImVec2(c_egl->width / 2 - 100, c_egl->heigth - 60));
        ImGui::SetNextWindowSize(ImVec2(400, 100));
        ImGui::Begin(oxorany("##dfdsfsdfdsdsf"), nullptr,
                     ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoBackground);
        if (ImGui::InvisibleButton(oxorany("##sfdsfsfsdfs"),
                                   ImGui::GetWindowSize()))
            open = !open;

        ImVec2 rectStart = ImVec2(c_egl->width / 2 - 100, c_egl->heigth - 20);
        ImVec2 rectEnd = ImVec2(c_egl->width / 2 + 100, c_egl->heigth - 10);
        float rectWidth = rectEnd.x - rectStart.x;

        ImGui::GetWindowDrawList()->AddRectFilled(
            rectStart,
            rectEnd,
            IM_COL32(255, 255, 255, 150),
            6.0f);

        ImVec2 indicatorPos = ImVec2(c_egl->width / 2, c_egl->heigth - 15);
        ImGui::GetWindowDrawList()->AddCircleFilled(
            indicatorPos,
            3.0f,
            IM_COL32(200, 200, 200, 200));

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(rectStart.x, rectStart.y),
            ImVec2(rectStart.x + 15, rectEnd.y),
            IM_COL32(255, 255, 255, 40),
            3.0f);

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(rectEnd.x - 15, rectStart.y),
            ImVec2(rectEnd.x, rectEnd.y),
            IM_COL32(255, 255, 255, 40),
            3.0f);

        ImGui::End();

        if (open && alpha < 1.f)
        {
            alpha += 0.025f;
        }
        else if (!open && alpha > 0.025f)
        {
            alpha -= 0.025f;
        }
    }
}