#include <list>
#include <vector>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "includes/imgui/imgui.h"
#include "includes/imgui/backends/imgui_impl_android.h"
#include "includes/imgui/backends/imgui_impl_opengl3.h"
#include <cstdio>
#include <cstdlib>
#include <link.h>
#include <elf.h>
#include <cinttypes>
#include <linux/elf.h>
#include "globals.hpp"
#include "src/menu/gui.h"
#include "includes/fonts/verdana.h"
#include "includes/fonts/smallest_pixel.h"
#include "src/menu/c_user_interface.hpp"
#include "src/features/esp.h"
#include "src/features/visual.h"
// vis 0x3A4E0 //ya 0x30EC8

bool egl_inited = false;
bool cmi;
using namespace ImGui;
void handle_touch()
{
    auto &io = ImGui::GetIO();
    int touch_count = c_methods ? c_methods->get_count() : 0;
    static bool touch_active = false;

    if (touch_count <= 0)
    {
        if (touch_active)
        {
            io.MouseDown[0] = false;
            touch_active = false;
        }
        return;
    }

    for (int i = 0; i < touch_count; i++)
    {
        auto it = c_methods->get_touch(i);
        auto phase = it.fields.m_Phase;

        float x = it.fields.m_Position.x;
        float y = io.DisplaySize.y - it.fields.m_Position.y;

        if (phase == TouchPhase::Began || phase == TouchPhase::Stationary || phase == TouchPhase::Moved)
        {
            io.MousePos = ImVec2(x, y);
            io.MouseDown[0] = true;
            touch_active = true;
        }
        else if (phase == TouchPhase::Ended || phase == TouchPhase::Canceled)
        {
            io.MouseDown[0] = false;
            touch_active = false;
        }
    }
}

ImFont *addFontFromMemory(void *font, int bytes, float size, float rasterizerMultiply = 1.5f)
{
    ImFontConfig font_cfg;
    font_cfg.SizePixels = size;
    font_cfg.OversampleH = font_cfg.OversampleV = 3;
    font_cfg.RasterizerMultiply = rasterizerMultiply;
    font_cfg.FontDataOwnedByAtlas = false;
    ImGuiIO &io = ImGui::GetIO();
    return io.Fonts->AddFontFromMemoryTTF(font, bytes, size, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
}

inline C_UserInterface ui;

void setup()
{
    ImGuiIO &io = GetIO();
    // ui.init();
    gui::font = addFontFromMemory(verdana, sizeof(verdana), 30.0f);
    gui::pixel = io.Fonts->AddFontFromMemoryTTF(smallest_pixel_data, sizeof(smallest_pixel_data), 10.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    // ui.init();
}
int (*orig_GfxDeviceGLES_PresentFrame)(uintptr_t instance);
int hk_GfxDeviceGLES_PresentFrame(uintptr_t instance)
{
    init();
    c_globals->init();
    auto width_fn = c_methods->get_width();
    auto heigh_fn = c_methods->get_heigth();

    if (!width_fn || !heigh_fn)
        return orig_GfxDeviceGLES_PresentFrame(instance);

    auto w = width_fn;
    auto h = heigh_fn;

    if (w <= 0 || h <= 0)
        return orig_GfxDeviceGLES_PresentFrame(instance);

    c_egl->width = w;
    c_egl->heigth = h;

    if (!egl_inited)
    {
        CreateContext();
        ImGuiIO &io = GetIO();
        io.DisplaySize = ImVec2(w, h);
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplAndroid_Init(NULL);
        setup();

        auto style = &ImGui::GetStyle();

        ImGui::StyleColorsDark();

        style->WindowBorderSize = 1.f;
        style->ChildBorderSize = 1.f;
        style->FrameBorderSize = 1.f;
        style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style->Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style->Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        style->Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_Button] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        style->Colors[ImGuiCol_Header] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.f);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style->Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
        style->Colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
        style->Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        style->Colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
        style->Colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        style->Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        style->Colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        style->Colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        style->WindowPadding = ImVec2(20, 20);
        style->FramePadding = ImVec2(4, 0);
        style->ItemSpacing = ImVec2(15, 15);
        style->ScrollbarRounding = 0;
        style->ScrollbarSize = 20;

        egl_inited = true;
    }

    ImGuiIO &io = GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(w, h);

    NewFrame();
    handle_touch();
    gui::render();
    //c_globals->update();
    c_visual->draw_hits();
    c_visual->hitmarker();
    c_esp->render();

    if (cmi)
    {
        io.MousePos = ImVec2(-1, -1);
        cmi = false;
    }

    EndFrame();
    Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

    return orig_GfxDeviceGLES_PresentFrame(instance);
}

EGLBoolean (*old_egl_swap_buffers)(EGLDisplay display, EGLSurface surface);
EGLBoolean hook_egl_swap_buffers(EGLDisplay display, EGLSurface surface)
{
    init();
    int (*width)() = (int (*)())(base + c_offsets->get_width);
    int (*heigh)() = (int (*)())(base + c_offsets->get_heigth);
    c_egl->width = width();
    c_egl->heigth = heigh();
    // c_globals->init();

    if (!egl_inited)
    {
        CreateContext();
        ImGuiIO &io = GetIO();
        io.DisplaySize = ImVec2(width(), heigh());
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplAndroid_Init(NULL);
        setup();
        auto style = &ImGui::GetStyle();

        ImGui::StyleColorsDark();

        style->WindowBorderSize = 1.f;
        style->ChildBorderSize = 1.f;
        style->FrameBorderSize = 1.f;
        style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style->Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        style->Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        style->Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style->Colors[ImGuiCol_Button] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        style->Colors[ImGuiCol_Header] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        style->Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
        style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.f);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style->Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
        style->Colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
        style->Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
        style->Colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
        style->Colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        style->Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style->Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        style->Colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        style->Colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        style->WindowPadding = ImVec2(20, 20);
        style->FramePadding = ImVec2(4, 0);
        style->ItemSpacing = ImVec2(15, 15);
        style->ScrollbarRounding = 0;
        style->ScrollbarSize = 20;
        egl_inited = true;
    }
    ImGuiIO &io = GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(width(), heigh());

    NewFrame();
    handle_touch();
    gui::render();
    // c_globals->update();

    if (cmi)
    {
        io.MousePos = ImVec2(-1, -1);
        cmi = false;
    }

    EndFrame();

    Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

    ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

    return old_egl_swap_buffers(display, surface);
}

bool validate_elf(uintptr_t address)
{
    Elf64_Ehdr *elfHeader = reinterpret_cast<Elf64_Ehdr *>(address);

    if (elfHeader->e_ident[EI_MAG0] == ELFMAG0 &&
        elfHeader->e_ident[EI_MAG1] == ELFMAG1 &&
        elfHeader->e_ident[EI_MAG2] == ELFMAG2 &&
        elfHeader->e_ident[EI_MAG3] == ELFMAG3)
    {
        return true;
    }

    return false;
}

uintptr_t library_paste_finding(const char *lib_name)
{
    uintptr_t addr{};
    char line[1024];
    uint64_t start{};
    uint64_t end{};
    char flags[5];
    char path[PATH_MAX];

    FILE *fp = fopen(oxorany("/proc/self/maps"), oxorany("r"));
    if (fp != nullptr)
    {
        while (fgets(line, sizeof(line), fp))
        {
            strcpy(path, "");
            sscanf(line, "%" PRIx64 "-%" PRIx64 " %s %*" PRIx64 " %*x:%*x %*u %s\n", &start, &end, flags, path);
#if defined(__aarch64__)
            if (strstr(flags, oxorany("r--p")) == 0)
                continue;
#endif
            if (strstr(path, lib_name))
            {
                if (validate_elf(start))
                {
                    addr = start;
                    break;
                }
            }
        }
        fclose(fp);
    }
    return addr;
}

#pragma once
#include <asm-generic/unistd.h>
#include <sys/stat.h>
#include "includes/egl/memory.hpp"
#include "includes/egl/maps.hpp"

// namespace menu_includes
// {
//     #include "includes/plthook.h"
// }

struct hook_info2
{
    void *ptr_addr;
    void *hook_addr;
    void *orig_addr;
    bool is_swap_hook;
};

std::vector<hook_info2 *> hooked_funcs2;

#define ALLIGN(addr) ((void *)((size_t)(addr) & ~(sysconf(_SC_PAGESIZE) - 1)))

int get_memory_permission(void *address)
{
    unsigned long addr = (unsigned long)address;
    FILE *fp;
    char buf[PATH_MAX];
    char perms[5];
    int bol = 1;

    fp = fopen("/proc/self/maps", "r");
    if (!fp)
        return 0;

    while (fgets(buf, PATH_MAX, fp))
    {
        unsigned long start, end;
        int eol = (strchr(buf, '\n') != NULL);
        if (bol)
        {
            if (!eol)
                bol = 0;
        }
        else
        {
            if (eol)
                bol = 1;
            continue;
        }

        if (sscanf(buf, "%lx-%lx %4s", &start, &end, perms) != 3)
            continue;
        if (start <= addr && addr < end)
        {
            int prot = 0;
            if (perms[0] == 'r')
                prot |= PROT_READ;
            else if (perms[0] != '-')
            {
                fclose(fp);
                return 0;
            }
            if (perms[1] == 'w')
                prot |= PROT_WRITE;
            else if (perms[1] != '-')
            {
                fclose(fp);
                return 0;
            }
            if (perms[2] == 'x')
                prot |= PROT_EXEC;
            else if (perms[2] != '-')
            {
                fclose(fp);
                return 0;
            }
            if (perms[3] != 'p')
            {
                fclose(fp);
                return 0;
            }
            if (perms[4] != '\0')
            {
                perms[4] = '\0';
                fclose(fp);
                return 0;
            }
            fclose(fp);
            return prot;
        }
    }
    fclose(fp);
    return 0;
}

namespace menu_includes
{
    void hook(void *address, void *replace, void **orig)
    {
        void *page = ALLIGN(address);
        int old_prot = get_memory_permission(page);
        if (old_prot == 0)
            old_prot = PROT_READ | PROT_EXEC;

        mprotect(page, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE);
        if (orig)
            *orig = *(void **)address;
        __atomic_store_n((void **)address, replace, __ATOMIC_SEQ_CST);
        mprotect(page, sysconf(_SC_PAGESIZE), old_prot);
    }
}

template <class h, class o>
void swap_ptr(void *addr, h hk, o orig)
{
    hook_info2 *info = new hook_info2();
    info->ptr_addr = addr;
    info->hook_addr = (void *)hk;
    info->orig_addr = *(void **)addr;
    info->is_swap_hook = true;
    hooked_funcs2.push_back(info);
    menu_includes::hook(addr, (void *)hk, (void **)orig);
}

bool is_valid_ptr(uintptr_t p)
{
    return p > 0x10000 && (p & 0xfff) != 0;
}

bool is_executable_address(void *ptr)
{
    if (!ptr)
        return false;
    Dl_info info{};
    if (dladdr(ptr, &info) == 0)
        return false;
    return info.dli_fbase != nullptr;
}

void init_render_hook()
{
    // TODO(0.39.2): GfxDeviceGLES PresentFrame slot — verify against live libil2cpp/libunity
    // if menu fails to draw. Old 0.36.x slot kept as fallback starting point.
    void **present_frame_ptr = (void **)(base + oxorany(0x7B5AD10));
    if (present_frame_ptr && *present_frame_ptr)
        swap_ptr(present_frame_ptr, hk_GfxDeviceGLES_PresentFrame, (void **)&orig_GfxDeviceGLES_PresentFrame);
}

#define _GNU_SOURCE
#include <cstdio>
#include <cstdlib>
#include <link.h>

struct dl_error
{
    const char *error;

    dl_error() { error = ("cannot find"); }
    dl_error(const char *e) { error = e; }
};

struct section_data
{
    const char *name;
    uintptr_t address;
    bool founded;

    section_data(const char *n) { name = n; }
};

class Library
{
private:
    const char *library_name;
    uintptr_t address;
    bool founded;
    std::vector<dl_error *> errors;

public:
    Library(const char *library_name);

    static int callback(struct dl_phdr_info *info, size_t size, void *data);

    virtual void GetLibrary();

    virtual uintptr_t GetAddress();

    virtual std::vector<dl_error *> GetErrors();

    virtual int phdr_iterator(int (*callback)(dl_phdr_info *info, size_t size, void *data), section_data *data);

    virtual bool Loaded()
    {
        return founded;
    }
};

int Library::callback(struct dl_phdr_info *info, size_t size, void *data)
{
    const char *name = info->dlpi_name;
    section_data *sectionData = (section_data *)data;
    if (strstr(name, sectionData->name))
    {
        sectionData->address = info->dlpi_addr;
        sectionData->founded = true;
        return 1;
    }

    return 0;
}

int Library::phdr_iterator(int (*callback)(dl_phdr_info *info, size_t size, void *data), section_data *data)
{
    return dl_iterate_phdr(callback, (void *)data);
}

void Library::GetLibrary()
{
    section_data *data = new section_data(this->library_name);

    if (!this->phdr_iterator(callback, data))
    {
        dl_error *error = new dl_error();
        this->errors.push_back(error);
    }
    else
    {
        this->address = data->address;
        this->founded = data->founded;
    }

    delete data;
}

Library::Library(const char *library_name)
{
    this->library_name = library_name;

    this->GetLibrary();
}

uintptr_t Library::GetAddress()
{
    return this->address;
}

std::vector<dl_error *> Library::GetErrors()
{
    return this->errors;
}

class il2cpp_t
{
    uintptr_t _address;

public:
    uintptr_t address()
    {
        if (!this->_address)
        {
            Library *libil2cpp = new Library(oxorany("libunity.so"));
            this->_address = libil2cpp->GetAddress();
            delete libil2cpp;
        }

        return this->_address;
    }

    bool is_loaded()
    {
        Library *libunity = new Library(oxorany("libunity.so"));
        bool is_loaded = libunity->Loaded();

        delete libunity;
        return is_loaded;
    }
};

il2cpp_t *_il2cpp;

#include <cstdio>
#include <cstdint>
#include <unistd.h>
#include <stdio.h>
#include "src/features/update.h"

static bool address_in_maps(uintptr_t addr)
{
    FILE *f = fopen(oxorany("/proc/self/maps"), "r");
    if (!f)
        return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f))
    {
        unsigned long s = 0, e = 0;
        if (sscanf(line, "%lx-%lx", &s, &e) == 2)
        {
            if ((uintptr_t)addr >= s && (uintptr_t)addr < e)
            {
                found = true;
                break;
            }
        }
    }
    fclose(f);
    return found;
}

void *entry()
{
    if (!_il2cpp)
        _il2cpp = new il2cpp_t();

    while (true)
    {
        if (_il2cpp->is_loaded())
        {
            base = _il2cpp->address();
            if (base > 0x100000000)
                break;
        }
        sleep(1);
    }

    if (base > 0)
    {
        init_render_hook();
        c_update->init();
    }

    sleep(4);
    pthread_exit(nullptr);
    return nullptr;
}

// #include "zygisk/zygisk_init.hpp"
// REGISTER_ZYGISK_MODULE(tenmi_zygisk)
#include "jni.h"
extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM *vm, void *key)
{
    // key 1337 is passed by injector
    if (key != (void *)1337)
        return JNI_VERSION_1_6;

    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) == JNI_OK)
    {
    }

    std::thread(entry).detach();

    return JNI_VERSION_1_6;
}