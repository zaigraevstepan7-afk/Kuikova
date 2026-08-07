#include "overlay.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <GLES3/gl3.h>
#include <android/log.h>
#include <cmath>
#include <mutex>

#define NOVA_LOG(...) __android_log_print(ANDROID_LOG_INFO, "nova", __VA_ARGS__)

namespace {
bool g_imgui = false;
std::mutex g_touch_mu;
float g_touch_x = 0, g_touch_y = 0;
bool g_touch_down = false;
bool g_touch_fresh = false;
NovaConfig g_cfg;
}

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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 6.0f;
    st.FrameRounding = 4.0f;
    st.WindowBorderSize = 1.0f;
    if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
        NOVA_LOG("ImGui_ImplOpenGL3_Init failed");
        return false;
    }
    g_imgui = true;
    NOVA_LOG("imgui ready");
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
    if (!cfg.esp_box && !cfg.esp_name && !cfg.esp_snapline) return;
    if (!st.has_matrix) return;

    int local_team = -1;
    for (const auto& p : st.players) {
        if (p.is_local) { local_team = p.team; break; }
    }

    for (const auto& p : st.players) {
        if (p.is_local) continue;
        if (cfg.esp_team_check && local_team >= 0 && p.team == local_team) continue;

        float fx, fy, hx, hy;
        if (!world_to_screen(st.view_proj, p.feet, sw, sh, fx, fy)) continue;
        if (!world_to_screen(st.view_proj, p.head, sw, sh, hx, hy)) continue;

        const float h = std::fabs(fy - hy);
        const float w = h * 0.45f;
        if (h < 4.0f) continue;

        const ImU32 col = IM_COL32(80, 220, 120, 230);
        if (cfg.esp_box) {
            dl->AddRect(ImVec2(hx - w * 0.5f, hy), ImVec2(hx + w * 0.5f, fy),
                        col, 0.0f, 0, cfg.box_thickness);
        }
        if (cfg.esp_snapline) {
            dl->AddLine(ImVec2(sw * 0.5f, sh), ImVec2(fx, fy), IM_COL32(80, 220, 120, 160), 1.0f);
        }
        if (cfg.esp_name) {
            const char* label = p.name.empty() ? "player" : p.name.c_str();
            dl->AddText(ImVec2(hx - w * 0.5f, hy - 14.0f), IM_COL32(240, 240, 240, 230), label);
        }
    }
}

void nova_overlay_frame(int width, int height, GameState& st) {
    if (width <= 0 || height <= 0) return;
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

    // Status strip
    ImGui::SetNextWindowPos(ImVec2(8, 8), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("##nova_status", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::Text("nova | %s | players:%d | matrix:%s | f:%d",
                st.status ? st.status : "?",
                static_cast<int>(st.players.size()),
                st.has_matrix ? "YES" : "NO",
                st.frame);
    ImGui::End();

    if (g_cfg.show_menu) {
        ImGui::SetNextWindowSize(ImVec2(320, 280), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(20, 60), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("nova", &g_cfg.show_menu)) {
            ImGui::TextUnformatted("Standoff 2 0.39.2");
            ImGui::Separator();
            ImGui::Checkbox("ESP box", &g_cfg.esp_box);
            ImGui::Checkbox("ESP name", &g_cfg.esp_name);
            ImGui::Checkbox("ESP snapline", &g_cfg.esp_snapline);
            ImGui::Checkbox("Team check", &g_cfg.esp_team_check);
            ImGui::SliderFloat("Thickness", &g_cfg.box_thickness, 1.0f, 4.0f);
            ImGui::Separator();
            ImGui::Text("il2cpp: %p", reinterpret_cast<void*>(st.il2cpp));
            ImGui::Text("manager: %p", reinterpret_cast<void*>(st.manager));
            ImGui::Text("local: %p", reinterpret_cast<void*>(st.local));
        }
        ImGui::End();
    }

    // Floating open button when menu closed
    if (!g_cfg.show_menu) {
        ImGui::SetNextWindowPos(ImVec2(12, static_cast<float>(height) * 0.35f), ImGuiCond_Always);
        ImGui::Begin("##nova_open", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("nova")) g_cfg.show_menu = true;
        ImGui::End();
    }

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    draw_esp(dl, st, static_cast<float>(width), static_cast<float>(height));

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
