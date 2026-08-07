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
bool g_touch_from_input = false;
bool g_touch_is_norm = false;
NovaConfig g_cfg;
int g_hook_mode = 0;
int g_esp_drawn = 0;
}

void nova_set_hook_mode(int mode) { g_hook_mode = mode; }
NovaConfig& nova_cfg() { return g_cfg; }

void nova_feed_touch(float x, float y, bool down) {
    std::lock_guard<std::mutex> lock(g_touch_mu);
    g_touch_x = x;
    g_touch_y = y;
    g_touch_down = down;
    g_touch_from_input = true;
    g_touch_is_norm = false;
}

void nova_feed_touch_norm(float nx, float ny, bool down) {
    std::lock_guard<std::mutex> lock(g_touch_mu);
    g_touch_x = nx;
    g_touch_y = ny;
    g_touch_down = down;
    g_touch_from_input = true;
    g_touch_is_norm = true;
}

bool nova_overlay_ensure_imgui() {
    if (g_imgui) return true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 4.0f;
    st.FrameRounding = 3.0f;
    st.WindowBorderSize = 1.0f;
    st.FramePadding = ImVec2(6, 4);
    st.ItemSpacing = ImVec2(6, 4);
    st.TouchExtraPadding = ImVec2(12, 12);
    // Compact — no giant ScaleAllSizes
    io.FontGlobalScale = 1.0f;

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

static void draw_esp(ImDrawList* dl, GameState& st, float sw, float sh) {
    auto& cfg = g_cfg;
    g_esp_drawn = 0;
    if (!cfg.esp_box && !cfg.esp_name && !cfg.esp_snapline) return;
    if (!st.has_matrix) return;

    int local_team = -1;
    for (const auto& p : st.players) {
        if (p.is_local) { local_team = p.team; break; }
    }

    for (const auto& p : st.players) {
        if (p.is_local) continue;
        if (cfg.esp_team_check && local_team >= 0 && p.team == local_team) continue;

        float fx = 0, fy = 0, hx = 0, hy = 0;
        const bool feet_ok = world_to_screen(st.view_proj, p.feet, sw, sh, fx, fy);
        bool head_ok = world_to_screen(st.view_proj, p.head, sw, sh, hx, hy);
        if (!feet_ok && !head_ok) continue;

        if (feet_ok && !head_ok) {
            hx = fx;
            hy = fy - std::max(28.f, sh * 0.06f);
        } else if (!feet_ok && head_ok) {
            fx = hx;
            fy = hy + std::max(28.f, sh * 0.06f);
        }

        float top = std::min(hy, fy);
        float bot = std::max(hy, fy);
        float h = bot - top;
        if (h < 8.f) { h = 40.f; top = fy - h; bot = fy; }
        if (h > sh * 1.5f) continue;

        const float mid_x = (fx + hx) * 0.5f;
        const float w = std::max(12.f, h * 0.45f);
        const float x1 = mid_x - w * 0.5f;
        const float x2 = mid_x + w * 0.5f;
        if (x2 < 0 || x1 > sw || bot < 0 || top > sh) continue;

        const ImU32 col = IM_COL32(60, 255, 110, 255);
        if (cfg.esp_box) {
            dl->AddRect(ImVec2(x1, top), ImVec2(x2, bot), col, 0.f, 0, cfg.box_thickness);
        }
        if (cfg.esp_snapline) {
            dl->AddLine(ImVec2(sw * 0.5f, sh), ImVec2(mid_x, bot),
                        IM_COL32(60, 255, 110, 180), 1.2f);
        }
        if (cfg.esp_name) {
            const char* label = p.name.empty() ? "?" : p.name.c_str();
            dl->AddText(ImVec2(x1, top - 14.f), IM_COL32(255, 255, 255, 255), label);
        }
        ++g_esp_drawn;
    }
}

void nova_overlay_frame(int width, int height, GameState& st) {
    if (width <= 1 || height <= 1) return;
    if (!nova_overlay_ensure_imgui()) return;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
    io.DeltaTime = 1.f / 60.f;

    // Melodium: only emit button events on edges (else checkboxes double-toggle)
    {
        std::lock_guard<std::mutex> lock(g_touch_mu);
        if (g_touch_from_input) {
            float mx = g_touch_x, my = g_touch_y;
            if (g_touch_is_norm) {
                mx = g_touch_x * (float)width;
                my = g_touch_y * (float)height;
            }
            static bool prev_down = false;
            static float prev_x = -1.f, prev_y = -1.f;
            io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
            if (mx != prev_x || my != prev_y || g_touch_down != prev_down)
                io.AddMousePosEvent(mx, my);
            if (g_touch_down != prev_down)
                io.AddMouseButtonEvent(0, g_touch_down);
            prev_down = g_touch_down;
            prev_x = mx;
            prev_y = my;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Compact status
    ImGui::SetNextWindowPos(ImVec2(8, 8), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("##st", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
    const char* hm = g_hook_mode == 1 ? "g" : (g_hook_mode == 2 ? "i" : "-");
    ImGui::Text("nova#%s | %s | p:%d esp:%d | m:%s | f:%d",
                hm, st.status ? st.status : "?",
                (int)st.players.size(), g_esp_drawn,
                st.has_matrix ? "Y" : "N", st.frame);
    ImGui::End();

    if (g_cfg.show_menu) {
        ImGui::SetNextWindowSize(ImVec2(220, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(8, 40), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("nova", &g_cfg.show_menu)) {
            ImGui::Checkbox("box", &g_cfg.esp_box);
            ImGui::Checkbox("name", &g_cfg.esp_name);
            ImGui::Checkbox("line", &g_cfg.esp_snapline);
            ImGui::Checkbox("team", &g_cfg.esp_team_check);
            ImGui::SliderFloat("w", &g_cfg.box_thickness, 1.f, 4.f, "%.1f");
            ImGui::Text("il2 %p", (void*)st.il2cpp);
            ImGui::Text("mgr %p", (void*)st.manager);
        }
        ImGui::End();
    } else {
        ImGui::SetNextWindowPos(ImVec2(8, 40), ImGuiCond_Always);
        ImGui::Begin("##op", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("nova", ImVec2(72, 36)))
            g_cfg.show_menu = true;
        ImGui::End();
    }

    draw_esp(ImGui::GetForegroundDrawList(), st, (float)width, (float)height);

    ImGui::Render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
