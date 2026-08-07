#include "overlay.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <GLES3/gl3.h>
#include <cmath>
#include <algorithm>
#include <mutex>

namespace {
bool g_imgui = false;
std::mutex g_touch_mu;
float g_touch_x = 0, g_touch_y = 0;
bool g_touch_down = false;
bool g_touch_fresh = false;
NovaConfig g_cfg;
int g_hook_mode = 0;
}

void nova_set_hook_mode(int mode) { g_hook_mode = mode; }

NovaConfig& nova_cfg() { return g_cfg; }

void nova_feed_touch(float x, float y, bool down) {
    std::lock_guard<std::mutex> lock(g_touch_mu);
    g_touch_x = x;
    g_touch_y = y;
    g_touch_down = down;
    g_touch_fresh = true;
}

bool nova_overlay_ensure_imgui() {
    if (g_imgui) return true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 6.0f;
    st.FrameRounding = 4.0f;
    st.WindowBorderSize = 1.0f;
    // GLES3 — matches Unity ES3 contexts on modern devices
    if (!ImGui_ImplOpenGL3_Init("#version 300 es")) return false;
    g_imgui = true;
    return true;
}

void nova_overlay_shutdown() {
    if (!g_imgui) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    g_imgui = false;
}

static void add_corner_box(ImDrawList* dl, float x1, float y1, float x2, float y2,
                           ImU32 col, float th) {
    const float l = std::min(x2 - x1, y2 - y1) * 0.25f;
    // TL
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1 + l, y1), col, th);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 + l), col, th);
    // TR
    dl->AddLine(ImVec2(x2, y1), ImVec2(x2 - l, y1), col, th);
    dl->AddLine(ImVec2(x2, y1), ImVec2(x2, y1 + l), col, th);
    // BL
    dl->AddLine(ImVec2(x1, y2), ImVec2(x1 + l, y2), col, th);
    dl->AddLine(ImVec2(x1, y2), ImVec2(x1, y2 - l), col, th);
    // BR
    dl->AddLine(ImVec2(x2, y2), ImVec2(x2 - l, y2), col, th);
    dl->AddLine(ImVec2(x2, y2), ImVec2(x2, y2 - l), col, th);
}

static void draw_esp(ImDrawList* dl, GameState& st, float sw, float sh) {
    auto& cfg = g_cfg;
    if (!cfg.esp_box && !cfg.esp_name && !cfg.esp_snapline) return;
    if (!st.has_matrix) return;

    int local_team = -1;
    for (const auto& p : st.players) {
        if (p.is_local) {
            local_team = p.team;
            break;
        }
    }

    int drawn = 0;
    for (const auto& p : st.players) {
        if (p.is_local) continue;
        if (cfg.esp_team_check && local_team >= 0 && p.team == local_team) continue;

        float fx = 0, fy = 0, hx = 0, hy = 0;
        const bool feet_ok = world_to_screen(st.view_proj, p.feet, sw, sh, fx, fy);
        bool head_ok = world_to_screen(st.view_proj, p.head, sw, sh, hx, hy);

        if (!feet_ok && !head_ok) continue;

        // Fallback: invent the missing point from box height estimate
        if (feet_ok && !head_ok) {
            hx = fx;
            hy = fy - (sh * 0.08f); // ~8% screen heuristic
            head_ok = true;
        } else if (!feet_ok && head_ok) {
            fx = hx;
            fy = hy + (sh * 0.08f);
        }

        // Screen Y grows downward; top = smaller y
        float top = std::min(hy, fy);
        float bot = std::max(hy, fy);
        float h = bot - top;
        if (h < 6.f) {
            // Degenerate — expand around feet
            h = 40.f;
            top = fy - h;
            bot = fy;
        }
        if (h > sh * 1.2f) continue;

        const float mid_x = (fx + hx) * 0.5f;
        const float w = h * 0.45f;
        const float x1 = mid_x - w * 0.5f;
        const float x2 = mid_x + w * 0.5f;

        // Off-screen cull (fully outside)
        if (x2 < 0 || x1 > sw || bot < 0 || top > sh) continue;

        const bool enemy = (local_team < 0) || (p.team != local_team);
        const ImU32 col = enemy ? IM_COL32(80, 220, 120, 240) : IM_COL32(80, 160, 255, 200);

        if (cfg.esp_box) {
            // Full rect + corner accents for visibility
            dl->AddRect(ImVec2(x1, top), ImVec2(x2, bot), col, 0.f, 0, cfg.box_thickness);
            add_corner_box(dl, x1, top, x2, bot, col, cfg.box_thickness + 0.5f);
        }
        if (cfg.esp_snapline) {
            dl->AddLine(ImVec2(sw * 0.5f, sh), ImVec2(mid_x, bot),
                        IM_COL32(80, 220, 120, 140), 1.0f);
        }
        if (cfg.esp_name) {
            const char* label = p.name.empty() ? "?" : p.name.c_str();
            dl->AddText(ImVec2(x1, top - 15.f), IM_COL32(245, 245, 245, 240), label);
        }
        ++drawn;
    }
    (void)drawn;
}

void nova_overlay_frame(int width, int height, GameState& st) {
    if (width <= 1 || height <= 1) return;
    if (!nova_overlay_ensure_imgui()) return;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DeltaTime = 1.0f / 60.0f;

    {
        std::lock_guard<std::mutex> lock(g_touch_mu);
        if (g_touch_fresh) {
            io.AddMousePosEvent(g_touch_x, g_touch_y);
            io.AddMouseButtonEvent(0, g_touch_down);
            g_touch_fresh = false;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(8, 8), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.50f);
    ImGui::Begin("##st", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    const char* hm = g_hook_mode == 1 ? "g" : (g_hook_mode == 2 ? "i" : "-");
    ImGui::Text("nova#%s | %s | p:%d | m:%s | f:%d",
                hm,
                st.status ? st.status : "?",
                static_cast<int>(st.players.size()),
                st.has_matrix ? "Y" : "N",
                st.frame);
    ImGui::End();

    if (g_cfg.show_menu) {
        ImGui::SetNextWindowSize(ImVec2(280, 220), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(20, 56), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("nova", &g_cfg.show_menu)) {
            ImGui::Checkbox("box", &g_cfg.esp_box);
            ImGui::Checkbox("name", &g_cfg.esp_name);
            ImGui::Checkbox("line", &g_cfg.esp_snapline);
            ImGui::Checkbox("team", &g_cfg.esp_team_check);
            ImGui::SliderFloat("w", &g_cfg.box_thickness, 1.0f, 4.0f);
        }
        ImGui::End();
    }

    if (!g_cfg.show_menu) {
        ImGui::SetNextWindowPos(ImVec2(12, static_cast<float>(height) * 0.35f), ImGuiCond_Always);
        ImGui::Begin("##op", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("nova")) g_cfg.show_menu = true;
        ImGui::End();
    }

    // Foreground so boxes stay above translucent menu chrome
    draw_esp(ImGui::GetForegroundDrawList(), st,
             static_cast<float>(width), static_cast<float>(height));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
