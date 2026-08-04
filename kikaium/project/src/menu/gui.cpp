#include "gui.h"
#include <globals.hpp>
#include "c_user_interface.hpp"
#include "includes/kikaium_touch.h"
#include <cmath>

float alpha = 0.0f;
inline C_UserInterface ui;

namespace gui
{
    __attribute__((used, retain)) static const char k_stux_contract[] =
        "xxx_contract:wm_click+egl+vmt+esp+bypass";

    void DrawGradientLine(ImDrawList *drawList, ImVec2 start, ImVec2 end, float startThickness, ImU32 startColor, ImU32 endColor, int segments = 20)
    {
        for (int i = 0; i < segments; i++)
        {
            float t0 = (float)i / segments;
            ImVec2 p0 = ImVec2(start.x + (end.x - start.x) * t0, start.y + (end.y - start.y) * t0);
            ImVec2 p1 = ImVec2(start.x + (end.x - start.x) * ((float)(i + 1) / segments),
                               start.y + (end.y - start.y) * ((float)(i + 1) / segments));
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
        float lineLength = (c_egl->heigth / 4.0f) / 2.f;
        float gap = 40.0f;
        ImU32 startColor = IM_COL32(255, 196, 72, 160);
        ImU32 endColor = IM_COL32(255, 196, 72, 15);
        DrawGradientLine(drawList, ImVec2(center.x, center.y - lineLength), ImVec2(center.x, center.y - gap), 2.5f, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x, center.y + lineLength), ImVec2(center.x, center.y + gap), 2.5f, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x - lineLength, center.y), ImVec2(center.x - gap, center.y), 2.5f, startColor, endColor);
        DrawGradientLine(drawList, ImVec2(center.x + lineLength, center.y), ImVec2(center.x + gap, center.y), 2.5f, startColor, endColor);
    }

    void DrawWatermark()
    {
        if (!g.b_watermark && !open)
            return;
        (void)k_stux_contract;

        const char *brand = oxorany("xxx");

        ImGui::SetNextWindowPos(ImVec2(16.f, 16.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);

        ImGui::Begin(oxorany("##stux_wm"), nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoNav);

        ImVec2 ts = ImGui::CalcTextSize(brand);
        float pad = 14.f;
        float width = ts.x + pad * 2.f + 8.f;
        float height = ts.y + pad * 2.f;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 rmin = p;
        ImVec2 rmax = ImVec2(p.x + width, p.y + height);

        dl->AddRectFilled(rmin, rmax, IM_COL32(0, 0, 0, 200), 0.f);
        dl->AddRect(rmin, rmax, IM_COL32(255, 196, 72, 220), 0.f, 0, 1.5f);
        dl->AddText(ImVec2(p.x + pad, p.y + pad), IM_COL32(255, 255, 255, 255), brand);

        ImGui::Dummy(ImVec2(width, height));
        ImGui::SetCursorScreenPos(rmin);
        if (ImGui::InvisibleButton(oxorany("##wm_click"), ImVec2(width, height)))
        {
            open = !open;
            g_menu_input_lock.store(4, std::memory_order_release);
        }
        // Keep watermark topmost for hit-test so it can close the menu
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
        ImGui::End();
    }

    void other()
    {
        const ImVec2 pos{}, size{(float)c_egl->width, (float)c_egl->heigth};
        ImGui::SetNextWindowPos(pos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(size, ImGuiCond_Always);
        ImGui::Begin(oxorany("##stux_overlay"), nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
        if (g.b_scope && c_player->local && c_player->weapon_parameters && c_globals->is_scoped())
            cross();
        ImGui::End();
    }

    void render()
    {
        other();
        if (open && alpha < 1.f)
            alpha += 0.06f;
        else if (!open && alpha > 0.02f)
            alpha -= 0.06f;
        if (alpha > 1.f)
            alpha = 1.f;
        if (alpha < 0.f)
            alpha = 0.f;
        // Menu first, watermark last → watermark stays clickable to close
        ui.render();
        DrawWatermark();
    }
}
