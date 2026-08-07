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
    g_touch_fresh = true;
    g_touch_is_norm = false;
}

void nova_feed_touch_norm(float nx, float ny, bool down) {
    std::lock_guard<std::mutex> lock(g_touch_mu);
    g_touch_x = nx;
    g_touch_y = ny;
    g_touch_down = down;
    g_touch_fresh = true;
    g_touch_is_norm = true;
}

bool nova_overlay_ensure_imgui() {
    if (g_imgui) return true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = false;
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 8.0f;
    st.FrameRounding = 6.0f;
    st.WindowBorderSize = 1.0f;
    st.FramePadding = ImVec2(10, 8);
    st.ItemSpacing = ImVec2(10, 8);
    // Bigger hit targets on phone
    st.TouchExtraPadding = ImVec2(8, 8);

    // Scale UI for typical phone DPI
    st.ScaleAllSizes(1.35f);
    io.FontGlobalScale = 1.25f;

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
    const float l = std::max(8.f, std::min(x2 - x1, y2 - y1) * 0.25f);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1 + l, y1), col, th);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 + l), col, th);
    dl->AddLine(ImVec2(x2, y1), ImVec2(x2 - l, y1), col, th);
    dl->AddLine(ImVec2(x2, y1), ImVec2(x2, y1 + l), col, th);
    dl->AddLine(ImVec2(x1, y2), ImVec2(x1 + l, y2), col, th);
    dl->AddLine(ImVec2(x1, y2), ImVec2(x1, y2 - l), col, th);
    dl->AddLine(ImVec2(x2, y2), ImVec2(x2 - l, y2), col, th);
    dl->AddLine(ImVec2(x2, y2), ImVec2(x2, y2 - l), col, th);
}

static void draw_esp(ImDrawList* dl, GameState& st, float sw, float sh) {
    auto& cfg = g_cfg;
    g_esp_drawn = 0;
    if (!cfg.esp_box && !cfg.esp_name && !cfg.esp_snapline) return;
    if (!st.has_matrix) return;

    int local_team = -1;
    for (const auto& p : st.players) {
        if (p.is_local) {
            local_team = p.team;
            break;
        }
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
            hy = fy - std::max(30.f, sh * 0.07f);
        } else if (!feet_ok && head_ok) {
            fx = hx;
            fy = hy + std::max(30.f, sh * 0.07f);
        }

        float top = std::min(hy, fy);
        float bot = std::max(hy, fy);
        float h = bot - top;
        if (h < 8.f) {
            h = 48.f;
            top = fy - h;
            bot = fy;
        }
        if (h > sh * 1.5f) continue;

        const float mid_x = (fx + hx) * 0.5f;
        const float w = std::max(14.f, h * 0.45f);
        const float x1 = mid_x - w * 0.5f;
        const float x2 = mid_x + w * 0.5f;
        if (x2 < -20 || x1 > sw + 20 || bot < -20 || top > sh + 20) continue;

        const ImU32 col = IM_COL32(60, 255, 110, 255);

        if (cfg.esp_box) {
            dl->AddRect(ImVec2(x1, top), ImVec2(x2, bot), col, 0.f, 0, cfg.box_thickness);
            add_corner_box(dl, x1, top, x2, bot, col, cfg.box_thickness + 0.5f);
            // subtle fill
            dl->AddRectFilled(ImVec2(x1, top), ImVec2(x2, bot), IM_COL32(60, 255, 110, 25));
        }
        if (cfg.esp_snapline) {
            dl->AddLine(ImVec2(sw * 0.5f, sh), ImVec2(mid_x, bot),
                        IM_COL32(60, 255, 110, 180), 1.5f);
        }
        if (cfg.esp_name) {
            const char* label = p.name.empty() ? "?" : p.name.c_str();
            dl->AddText(ImVec2(x1, top - 16.f), IM_COL32(255, 255, 255, 255), label);
        }
        ++g_esp_drawn;
    }
}

void nova_overlay_frame(int width, int height, GameState& st) {
    if (width <= 1 || height <= 1) return;
    if (!nova_overlay_ensure_imgui()) return;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
    io.DeltaTime = 1.0f / 60.0f;

    {
        std::lock_guard<std::mutex> lock(g_touch_mu);
        if (g_touch_fresh) {
            float mx = g_touch_x;
            float my = g_touch_y;
            if (g_touch_is_norm) {
                mx = g_touch_x * static_cast<float>(width);
                my = g_touch_y * static_cast<float>(height);
            }
            io.AddMousePosEvent(mx, my);
            io.AddMouseButtonEvent(0, g_touch_down);
            g_touch_fresh = false;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Status — always visible so we know overlay is alive
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGui::Begin("##st", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoInputs);
    const char* hm = g_hook_mode == 1 ? "g" : (g_hook_mode == 2 ? "i" : "-");
    ImGui::Text("nova#%s | %s | p:%d esp:%d | m:%s | f:%d",
                hm,
                st.status ? st.status : "?",
                static_cast<int>(st.players.size()),
                g_esp_drawn,
                st.has_matrix ? "Y" : "N",
                st.frame);
    ImGui::End();

    if (g_cfg.show_menu) {
        ImGui::SetNextWindowSize(ImVec2(320, 300), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(16, 70), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("nova", &g_cfg.show_menu)) {
            ImGui::TextUnformatted("ESP");
            ImGui::Separator();
            ImGui::Checkbox("box", &g_cfg.esp_box);
            ImGui::Checkbox("name", &g_cfg.esp_name);
            ImGui::Checkbox("line", &g_cfg.esp_snapline);
            ImGui::Checkbox("team check", &g_cfg.esp_team_check);
            ImGui::SliderFloat("thickness", &g_cfg.box_thickness, 1.0f, 5.0f);
            ImGui::Separator();
            ImGui::Text("il2cpp %p", reinterpret_cast<void*>(st.il2cpp));
            ImGui::Text("mgr %p", reinterpret_cast<void*>(st.manager));
            ImGui::Text("local %p", reinterpret_cast<void*>(st.local));
            ImGui::Text("players %d  drawn %d",
                        static_cast<int>(st.players.size()), g_esp_drawn);
        }
        ImGui::End();
    } else {
        ImGui::SetNextWindowPos(ImVec2(14, static_cast<float>(height) * 0.4f), ImGuiCond_Always);
        ImGui::Begin("##op", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button(" MENU ", ImVec2(120, 48)))
            g_cfg.show_menu = true;
        ImGui::End();
    }

    draw_esp(ImGui::GetForegroundDrawList(), st,
             static_cast<float>(width), static_cast<float>(height));

    ImGui::Render();

    // Ensure default framebuffer for present
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
