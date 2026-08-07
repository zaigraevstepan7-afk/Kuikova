#include "overlay.hpp"
#include "chams.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <GLES3/gl3.h>
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
    st.ScaleAllSizes(3.0f);
    io.FontGlobalScale = 3.0f;

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

void nova_overlay_frame(int width, int height, GameState& st) {
    if (width <= 1 || height <= 1) return;
    if (!nova_overlay_ensure_imgui()) return;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
    io.DeltaTime = 1.f / 60.f;

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

    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("##st", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
    const char* hm = g_hook_mode == 1 ? "g" : (g_hook_mode == 2 ? "i" : "-");
    const char* ch = chams_hook_ok() == 1 ? "a64" : (chams_hook_ok() == 2 ? "dob" : "-");
    ImGui::Text("nova#%s | %s | ch:%s/%s | a:%d | f:%d",
                hm, st.status ? st.status : "?",
                ch, chams_status(), chams_applied(), st.frame);
    ImGui::End();

    if (g_cfg.show_menu) {
        ImGui::SetNextWindowSize(ImVec2(720, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(16, 100), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("nova", &g_cfg.show_menu)) {
            auto& c = chams_cfg();
            ImGui::Checkbox("chams", &c.enabled);
            ImGui::Checkbox("team", &c.team_check);
            ImGui::Checkbox("local", &c.local_chams);
            const char* mats[] = {
                "Diffuse", "Internal", "Glass", "Metal", "Transparent"
            };
            ImGui::Combo("mat", &c.material, mats, 5);
            ImGui::ColorEdit4("color", c.color, ImGuiColorEditFlags_NoInputs);
            ImGui::Text("il2 %p", (void*)st.il2cpp);
            ImGui::Text("uni %p", (void*)st.unity);
            ImGui::Text("mgr %p", (void*)st.manager);
        }
        ImGui::End();
    } else {
        ImGui::SetNextWindowPos(ImVec2(16, 100), ImGuiCond_Always);
        ImGui::Begin("##op", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("nova", ImVec2(216, 108)))
            g_cfg.show_menu = true;
        ImGui::End();
    }

    ImGui::Render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
