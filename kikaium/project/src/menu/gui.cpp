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
        "xxxpastuxxx_contract:wm_click+egl+vmt+esp";

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
        float startThickness = 2.5f;
        ImU32 startColor = IM_COL32(61, 255, 181, 170);
        ImU32 endColor = IM_COL32(61, 255, 181, 15);
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
        (void)k_stux_contract;

        const char *brand = oxorany("xxxpastuxxx");
        const char *tg = oxorany("t.me/xxxstuxxx");
        const char *hint = open ? oxorany("tap — close") : oxorany("tap — open");

        const float t = (float)ImGui::GetTime();
        const int pulse_a = 140 + (int)(50.f * sinf(t * 1.8f));

        ImGui::SetNextWindowPos(ImVec2(14.f, 14.f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        if (!open)
            ImGui::SetNextWindowFocus();

        ImGui::Begin(oxorany("##stux_wm"), nullptr,
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
        ImVec2 tg_sz = ImGui::CalcTextSize(tg);
        ImVec2 hint_sz = ImGui::CalcTextSize(hint);
        float text_w = brand_sz.x;
        if (tg_sz.x > text_w) text_w = tg_sz.x;
        if (hint_sz.x > text_w) text_w = hint_sz.x;
        float width = text_w + 52.f;
        float height = brand_sz.y + tg_sz.y + hint_sz.y + 28.f;

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 rmin = p;
        ImVec2 rmax = ImVec2(p.x + width, p.y + height);

        // Flat ink plate — no glass, no copper spine (new look)
        dl->AddRectFilled(rmin, rmax, IM_COL32(8, 12, 14, 235), 0.f);
        dl->AddRect(rmin, rmax, IM_COL32(40, 55, 50, 220), 0.f, 0, 1.0f);
        // Top mint hairline
        dl->AddRectFilled(rmin, ImVec2(rmax.x, rmin.y + 2.f), IM_COL32(61, 255, 181, pulse_a));
        // Right mint tick
        dl->AddRectFilled(ImVec2(rmax.x - 2.f, rmin.y + 8.f), ImVec2(rmax.x, rmax.y - 8.f),
                          IM_COL32(61, 255, 181, 200));

        float y = p.y + 8.f;
        dl->AddText(ImVec2(p.x + 14.f, y), IM_COL32(230, 245, 238, 255), brand);
        y += brand_sz.y + 2.f;
        dl->AddText(ImVec2(p.x + 14.f, y), IM_COL32(61, 255, 181, 230), tg);
        y += tg_sz.y + 2.f;
        dl->AddText(ImVec2(p.x + 14.f, y), IM_COL32(110, 130, 120, 255), hint);

        ImGui::Dummy(ImVec2(width, height));
        ImGui::SetCursorScreenPos(rmin);
        bool clicked = ImGui::InvisibleButton(oxorany("##wm_click"), ImVec2(width, height));
        if (!clicked && kik_input::consume_tap_in_rect(rmin.x, rmin.y, rmax.x, rmax.y))
            clicked = true;

        if (clicked)
        {
            open = !open;
            g_menu_input_lock.store(8, std::memory_order_release);
        }

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
