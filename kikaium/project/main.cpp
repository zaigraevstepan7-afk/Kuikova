// ============================================================================
// Kikaium - Halalium-architecture shell + Kikaium features
// Render: eglSwapBuffers GOT hook (Halalium uses Dobby on same symbol)
// Menu:   ##watermark / ##wm_click
// Update: tools/halalium_emu/update.sh  (see UPDATE.md)
// ============================================================================

#include <list>
#include <vector>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <pthread.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/input.h>
#include <android/log.h>
#include "includes/imgui/imgui.h"
#include "includes/imgui/backends/imgui_impl_android.h"
#include "includes/imgui/backends/imgui_impl_opengl3.h"
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <link.h>
#include <elf.h>
#include <cinttypes>
#include <linux/elf.h>
#include "globals.hpp"
// Do NOT include <fcntl.h> here - globals.hpp defines `bool open`, which collides.
#include "includes/halalium_hooks.h"
#include "includes/a64_inline_hook.h"
#include "includes/kikaium_touch.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <mutex>
#include "src/menu/gui.h"
#include "includes/fonts/verdana.h"
#include "includes/fonts/smallest_pixel.h"
#include "src/menu/c_user_interface.hpp"
#include "src/features/esp.h"
#include "src/features/visual.h"
#include "src/features/update.h"
// vis 0x3A4E0 //ya 0x30EC8

bool egl_inited = false;
bool cmi;
using namespace ImGui;

// ---------------------------------------------------------------------------
// Touch — Halalium input_consume_cb @0x1d760c:
//   InputConsumer::consume → AInputEvent_getType / AMotionEvent_getAction/getX/getY
//   (UND imports in libhalalium.so — same NDK APIs, NOT raw C++ vtable).
// Unity GetTouch is backup after SDK ready.
// Applied to ImGui BEFORE NewFrame (AddMouse*Event).
// ---------------------------------------------------------------------------
struct TouchFeed
{
    float x = 0.f;
    float y = 0.f;
    bool down = false;
    bool from_input = false;
    bool prev_down = false;
    bool tap_edge = false; // false→true transition this update
};
static TouchFeed g_touch;
static std::mutex g_touch_mu;

using ain_get_type_fn = int32_t (*)(const AInputEvent *);
using am_get_action_fn = int32_t (*)(const AInputEvent *);
using am_get_xy_fn = float (*)(const AInputEvent *, size_t);
static ain_get_type_fn g_ain_get_type = nullptr;
static am_get_action_fn g_am_get_action = nullptr;
static am_get_xy_fn g_am_get_x = nullptr;
static am_get_xy_fn g_am_get_y = nullptr;

using consume_fn = int32_t (*)(void *thiz, void *factory, bool consumeBatches,
                               int64_t frameTime, uint32_t *outSeq, AInputEvent **outEvent);
static consume_fn old_input_consume = nullptr;

static bool resolve_ainput_apis()
{
    if (g_ain_get_type && g_am_get_action && g_am_get_x && g_am_get_y)
        return true;
    void *lib = dlopen("libandroid.so", RTLD_NOW);
    if (!lib)
        return false;
    g_ain_get_type = (ain_get_type_fn)dlsym(lib, "AInputEvent_getType");
    g_am_get_action = (am_get_action_fn)dlsym(lib, "AMotionEvent_getAction");
    g_am_get_x = (am_get_xy_fn)dlsym(lib, "AMotionEvent_getX");
    g_am_get_y = (am_get_xy_fn)dlsym(lib, "AMotionEvent_getY");
    // Fallback to linked NDK symbols if dlsym missed (same as Halalium UND).
    if (!g_ain_get_type)
        g_ain_get_type = &AInputEvent_getType;
    if (!g_am_get_action)
        g_am_get_action = &AMotionEvent_getAction;
    if (!g_am_get_x)
        g_am_get_x = &AMotionEvent_getX;
    if (!g_am_get_y)
        g_am_get_y = &AMotionEvent_getY;
    return g_ain_get_type && g_am_get_action && g_am_get_x && g_am_get_y;
}

static void touch_store(float x, float y, bool down, bool from_input)
{
    std::lock_guard<std::mutex> lock(g_touch_mu);
    g_touch.x = x;
    g_touch.y = y;
    g_touch.tap_edge = down && !g_touch.down;
    g_touch.prev_down = g_touch.down;
    g_touch.down = down;
    if (from_input)
        g_touch.from_input = true;
}

bool kik_input::consume_tap_in_rect(float x0, float y0, float x1, float y1)
{
    std::lock_guard<std::mutex> lock(g_touch_mu);
    if (!g_touch.tap_edge)
        return false;
    const float x = g_touch.x;
    const float y = g_touch.y;
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    g_touch.tap_edge = false; // consume once
    return true;
}

static int32_t hk_input_consume(void *thiz, void *factory, bool consumeBatches,
                                int64_t frameTime, uint32_t *outSeq, AInputEvent **outEvent)
{
    int32_t status = old_input_consume
                         ? old_input_consume(thiz, factory, consumeBatches, frameTime, outSeq, outEvent)
                         : -1;
    // Halalium: only when consume OK (status==0) and outEvent non-null with event
    if (status != 0 || !outEvent || !*outEvent)
        return status;
    if (!g_ain_get_type || !g_am_get_action || !g_am_get_x || !g_am_get_y)
        return status;

    AInputEvent *ev = *outEvent;
    // AINPUT_EVENT_TYPE_MOTION == 2 (Halalium cmp w0, #2)
    if (g_ain_get_type(ev) != AINPUT_EVENT_TYPE_MOTION)
        return status;

    const int32_t action = g_am_get_action(ev);
    // Halalium: ands w8, w21, #0xfd — only DOWN(0)/UP(1) flip down-bit; MOVE keeps prior
    const int32_t masked_fd = action & 0xfd;
    const float x = g_am_get_x(ev, 0);
    const float y = g_am_get_y(ev, 0);

    {
        std::lock_guard<std::mutex> lock(g_touch_mu);
        bool down = g_touch.down;
        if (masked_fd == 0)
            down = true; // ACTION_DOWN
        else if (masked_fd == 1)
            down = false; // ACTION_UP
        g_touch.tap_edge = down && !g_touch.down;
        g_touch.prev_down = g_touch.down;
        g_touch.x = x;
        g_touch.y = y;
        g_touch.down = down;
        g_touch.from_input = true;
    }
    return status;
}

static bool install_input_consume_hook()
{
    if (!Offsets::Hook::use_input_consume)
    {
        LOGI("InputConsumer hook skipped (flag off)");
        return false;
    }
    static bool tried = false;
    if (tried)
        return old_input_consume != nullptr;
    tried = true;

    if (!resolve_ainput_apis())
    {
        LOGI("AInputEvent APIs missing — InputConsumer skipped");
        return false;
    }

    void *lib = dlopen("libinput.so", RTLD_NOW);
    if (!lib)
        lib = dlopen("libandroid.so", RTLD_NOW);
    void *sym = lib ? dlsym(lib, "_ZN7android13InputConsumer7consumeEPNS_26InputEventFactoryInterfaceEblPjPPNS_10InputEventE")
                    : nullptr;
    if (!sym)
    {
        LOGI("InputConsumer::consume symbol missing");
        return false;
    }
    // Prefer a64 for InputConsumer (menu tap). Dobby fallback. Always UNTRACKED.
    void *tramp = nullptr;
    if (a64hook::install(sym, (void *)hk_input_consume, &tramp) && tramp)
    {
        old_input_consume = (decltype(old_input_consume))tramp;
        LOGI("InputConsumer::consume a64 @%p (untracked)", sym);
        return true;
    }
    void *orig = nullptr;
    if (hhooks::install_untracked(sym, (void *)hk_input_consume, &orig) && orig)
    {
        old_input_consume = (decltype(old_input_consume))orig;
        LOGI("InputConsumer::consume DobbyHook @%p (untracked)", sym);
        return true;
    }
    LOGI("InputConsumer::consume hook failed @%p", sym);
    return false;
}

void handle_touch()
{
    auto &io = ImGui::GetIO();

    // 1) Halalium InputConsumer feed (preferred)
    {
        float x, y;
        bool down, from_input;
        {
            std::lock_guard<std::mutex> lock(g_touch_mu);
            x = g_touch.x;
            y = g_touch.y;
            down = g_touch.down;
            from_input = g_touch.from_input;
        }
        if (from_input)
        {
            // Only emit BUTTON edges — repeating AddMouseButtonEvent(true) every
            // frame makes checkboxes/tabs toggle on press AND release.
            static bool prev_down = false;
            static float prev_x = -1.f, prev_y = -1.f;
            io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
            if (x != prev_x || y != prev_y || down != prev_down)
                io.AddMousePosEvent(x, y);
            if (down != prev_down)
                io.AddMouseButtonEvent(0, down);
            prev_down = down;
            prev_x = x;
            prev_y = y;
            return;
        }
    }

    // 2) UnityEngine.Input backup (MethodInfo* last; icalls accept nullptr)
    if (!c_methods || !c_methods->get_count || !c_methods->get_touch)
        return;
    if (!g_sdk_ready.load(std::memory_order_acquire))
        return;

    int touch_count = 0;
    touch_count = c_methods->get_count();
    static bool touch_active = false;

    if (touch_count <= 0)
    {
        if (touch_active)
        {
            io.AddMouseButtonEvent(0, false);
            touch_active = false;
            float lx, ly;
            {
                std::lock_guard<std::mutex> lock(g_touch_mu);
                lx = g_touch.x;
                ly = g_touch.y;
            }
            touch_store(lx, ly, false, false);
        }
        return;
    }

    // Clamp - garbage count from bad resolve would crash
    if (touch_count > 8)
        touch_count = 8;

    for (int i = 0; i < touch_count; i++)
    {
        auto it = c_methods->get_touch(i);
        auto phase = it.fields.m_Phase;

        float x = it.fields.m_Position.x;
        float y = io.DisplaySize.y - it.fields.m_Position.y;

        if (phase == TouchPhase::Began || phase == TouchPhase::Stationary || phase == TouchPhase::Moved)
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
            io.AddMousePosEvent(x, y);
            if (!touch_active)
                io.AddMouseButtonEvent(0, true);
            touch_active = true;
            touch_store(x, y, true, false);
        }
        else if (phase == TouchPhase::Ended || phase == TouchPhase::Canceled)
        {
            io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
            io.AddMousePosEvent(x, y);
            if (touch_active)
                io.AddMouseButtonEvent(0, false);
            touch_active = false;
            touch_store(x, y, false, false);
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
// Halalium-compat: PresentFrame path removed (dead on 0.39.2).
// Render + menu live entirely on eglSwapBuffers (see hook_egl_swap_buffers).

EGLBoolean (*old_egl_swap_buffers)(EGLDisplay display, EGLSurface surface);

static void apply_imgui_style()
{
    auto style = &ImGui::GetStyle();
    ImGui::StyleColorsDark();

    // Melodium / Kikaium own look: charcoal + copper (not Halalium grey, not teal clone).
    const ImVec4 copper = ImVec4(0.91f, 0.66f, 0.35f, 1.00f);
    const ImVec4 copper_dim = ImVec4(0.55f, 0.38f, 0.18f, 1.00f);
    const ImVec4 bg = ImVec4(0.055f, 0.058f, 0.065f, 0.98f);
    const ImVec4 bg2 = ImVec4(0.08f, 0.085f, 0.095f, 1.00f);

    style->WindowBorderSize = 1.f;
    style->ChildBorderSize = 1.f;
    style->FrameBorderSize = 0.f;
    style->WindowRounding = 0.f;
    style->ChildRounding = 0.f;
    style->FrameRounding = 2.f;
    style->GrabRounding = 2.f;
    style->ScrollbarRounding = 0.f;
    style->ScrollbarSize = 12.f;
    style->WindowPadding = ImVec2(12, 12);
    style->FramePadding = ImVec2(8, 5);
    style->ItemSpacing = ImVec2(12, 11);

    style->Colors[ImGuiCol_Text] = ImVec4(0.93f, 0.91f, 0.88f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.44f, 0.42f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = bg;
    style->Colors[ImGuiCol_ChildBg] = bg2;
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.09f, 0.10f, 0.96f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.22f, 0.23f, 0.25f, 1.00f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.12f, 0.14f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.15f, 0.13f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = copper_dim;
    style->Colors[ImGuiCol_TitleBg] = bg;
    style->Colors[ImGuiCol_TitleBgActive] = copper_dim;
    style->Colors[ImGuiCol_TitleBgCollapsed] = bg;
    style->Colors[ImGuiCol_MenuBarBg] = bg2;
    style->Colors[ImGuiCol_ScrollbarBg] = bg;
    style->Colors[ImGuiCol_ScrollbarGrab] = copper_dim;
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = copper;
    style->Colors[ImGuiCol_ScrollbarGrabActive] = copper;
    style->Colors[ImGuiCol_CheckMark] = copper;
    style->Colors[ImGuiCol_SliderGrab] = copper;
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 0.78f, 0.45f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = copper_dim;
    style->Colors[ImGuiCol_ButtonActive] = copper;
    style->Colors[ImGuiCol_Header] = ImVec4(0.55f, 0.38f, 0.18f, 0.45f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.48f, 0.22f, 0.55f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.62f, 0.28f, 0.65f);
    style->Colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.26f, 0.22f, 0.80f);
    style->Colors[ImGuiCol_SeparatorHovered] = copper;
    style->Colors[ImGuiCol_SeparatorActive] = copper;
    style->Colors[ImGuiCol_ResizeGrip] = copper_dim;
    style->Colors[ImGuiCol_ResizeGripHovered] = copper;
    style->Colors[ImGuiCol_ResizeGripActive] = copper;
    style->Colors[ImGuiCol_Tab] = bg2;
    style->Colors[ImGuiCol_TabHovered] = copper_dim;
    style->Colors[ImGuiCol_TabSelected] = copper_dim;
    style->Colors[ImGuiCol_TabSelectedOverline] = copper;
    style->Colors[ImGuiCol_TabDimmed] = bg;
    style->Colors[ImGuiCol_TabDimmedSelected] = copper_dim;
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.91f, 0.66f, 0.35f, 0.35f);
    style->Colors[ImGuiCol_NavCursor] = copper;
}

// Halalium-style path: draw on eglSwapBuffers (dlsym + hook), query surface size like Halalium.
EGLBoolean hook_egl_swap_buffers(EGLDisplay display, EGLSurface surface)
{
    EGLint w = 0, h = 0;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    if (w <= 0 || h <= 0)
        return old_egl_swap_buffers ? old_egl_swap_buffers(display, surface) : EGL_FALSE;

    c_egl->width = w;
    c_egl->heigth = h;

    // NEVER call ::init()/img_to_asm from the GL thread every frame - that was
    // crashing right after ImGui inited (il2cpp_domain_get via bad/racy base).
    // Touch/SDK pointers are wired once from update::init (soft).

    if (!egl_inited)
    {
        CreateContext();
        ImGuiIO &io = GetIO();
        io.DisplaySize = ImVec2((float)w, (float)h);
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplAndroid_Init(NULL);
        setup();
        apply_imgui_style();
        egl_inited = true;
        LOGI("ImGui egl inited %dx%d", w, h);
    }

    ImGuiIO &io = GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);

    // Touch MUST be queued before ImGui::NewFrame (ImGui 1.87+ event API)
    handle_touch();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(w, h);
    NewFrame();
    gui::render();

    // Overlays after SDK ready
    const bool sdk = g_sdk_ready.load(std::memory_order_acquire);
    if (sdk)
    {
        if (c_update)
            c_update->tick_lobby_cleanup();
        if (g.b_fov_check && c_egl && c_egl->width > 0 && c_egl->heigth > 0)
        {
            ImDrawList *dl = ImGui::GetForegroundDrawList();
            const ImVec2 c(c_egl->width * 0.5f, c_egl->heigth * 0.5f);
            const float r = (g.f_fov_check / 180.f) * (c_egl->heigth * 0.5f);
            const ImU32 col = ImGui::ColorConvertFloat4ToU32(
                ImVec4(g.m_fov_color[0], g.m_fov_color[1], g.m_fov_color[2], g.m_fov_color[3]));
            dl->AddCircle(c, r, col, 64, 1.5f);
        }
        if (c_esp)
            c_esp->render();
    }

    // Status bar LAST (after menu/ESP/watermark) — always visible when ImGui lives
    if (c_esp)
    {
        c_esp->dbg_sdk.store(sdk ? 1 : 0, std::memory_order_relaxed);
        c_esp->draw_status();
    }

    if (cmi)
    {
        io.MousePos = ImVec2(-1, -1);
        cmi = false;
    }

    EndFrame();
    Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

    return old_egl_swap_buffers ? old_egl_swap_buffers(display, surface) : EGL_FALSE;
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

// Halalium: dlsym/DobbySymbolResolver(libEGL, eglSwapBuffers) + DobbyHook(symbol).
// a64hook emergency fallback declared via includes/a64_inline_hook.h (top).

static bool hook_egl_got_slots(void *symbol, void *replacement, void **out_orig)
{
    if (!symbol || !replacement)
        return false;

    FILE *f = fopen(oxorany("/proc/self/maps"), oxorany("r"));
    if (!f)
        return false;

    char line[512];
    int hooked = 0;
    while (fgets(line, sizeof(line), f))
    {
        uintptr_t start = 0, end = 0;
        char perms[8]{}, path[256]{};
        if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %7s %*s %*s %*s %255s", &start, &end, perms, path) < 3)
            continue;
        if (perms[0] != 'r' || perms[1] != 'w')
            continue;

        // Scan all writable maps - Unity GOT often lives in anon segments.
        for (uintptr_t p = start; p + sizeof(void *) <= end; p += sizeof(void *))
        {
            void *val = *(void **)p;
            if (val != symbol)
                continue;
            if (out_orig && !*out_orig)
                *out_orig = symbol;
            void *discard = nullptr;
            swap_ptr((void *)p, replacement, &discard);
            hooked++;
        }
    }
    fclose(f);
    return hooked > 0;
}

void init_render_hook()
{
    static bool egl_hooked = false;
    if (egl_hooked)
        return;

    // Absolute trampolines for any Dobby game hooks installed later
    dobby_set_near_trampoline(false);

    void *egl = dlopen(oxorany("libEGL.so"), RTLD_NOW);
    void *sym = egl ? dlsym(egl, oxorany("eglSwapBuffers")) : nullptr;
    if (!sym)
        sym = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (!sym)
    {
        LOGI("eglSwapBuffers resolve failed");
        return;
    }

    auto finish_ok = [&](void *orig, const char *how) {
        old_egl_swap_buffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))orig;
        egl_hooked = true;
        LOGI("eglSwapBuffers hook OK via %s sym=%p orig=%p", how, sym, orig);
        install_input_consume_hook();
    };

    // MENU CRITICAL: prefer a64 inline for egl (stable on OnePlus / AndKitty).
    // Halalium uses Dobby here too, but broken Dobby trampolines = no watermark.
    // Game RVAs + getrr still use Dobby (hide profile). egl/input stay UNTRACKED either way.
    void *tramp = nullptr;
    if (a64hook::install(sym, (void *)hook_egl_swap_buffers, &tramp) && tramp)
    {
        finish_ok(tramp, "a64");
        return;
    }
    LOGI("egl a64 failed — trying Dobby");

    void *orig = nullptr;
    if (hhooks::install_untracked(sym, (void *)hook_egl_swap_buffers, &orig) && orig)
    {
        finish_ok(orig, "Dobby");
        return;
    }

    LOGI("egl Dobby failed — GOT scan");
    old_egl_swap_buffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))sym;
    if (hook_egl_got_slots(sym, (void *)hook_egl_swap_buffers, (void **)&old_egl_swap_buffers))
    {
        egl_hooked = true;
        LOGI("eglSwapBuffers GOT hook OK sym=%p", sym);
        install_input_consume_hook();
        return;
    }

    std::thread([sym]() {
        dobby_set_near_trampoline(false);
        for (int i = 0; i < 40; i++)
        {
            void *t = nullptr;
            if (a64hook::install(sym, (void *)hook_egl_swap_buffers, &t) && t)
            {
                old_egl_swap_buffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))t;
                egl_hooked = true;
                LOGI("eglSwapBuffers a64 OK (delayed)");
                install_input_consume_hook();
                break;
            }
            void *o = nullptr;
            if (hhooks::install_untracked(sym, (void *)hook_egl_swap_buffers, &o) && o)
            {
                old_egl_swap_buffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))o;
                egl_hooked = true;
                LOGI("eglSwapBuffers Dobby OK (delayed)");
                install_input_consume_hook();
                break;
            }
            old_egl_swap_buffers = (EGLBoolean(*)(EGLDisplay, EGLSurface))sym;
            if (hook_egl_got_slots(sym, (void *)hook_egl_swap_buffers, (void **)&old_egl_swap_buffers))
            {
                egl_hooked = true;
                LOGI("eglSwapBuffers GOT OK (delayed)");
                install_input_consume_hook();
                break;
            }
            sleep(1);
        }
    }).detach();
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

#include "includes/module_base.h"

class il2cpp_t
{
    uintptr_t _address;

public:
    uintptr_t address()
    {
        if (!this->_address)
            this->_address = resolve_il2cpp_base();
        return this->_address;
    }

    bool is_loaded()
    {
        return resolve_il2cpp_base() != 0;
    }
};

il2cpp_t *_il2cpp;

#include <cstdio>
#include <cstdint>
#include <unistd.h>
#include <stdio.h>

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
    // Match Halalium thread naming (Hooks / Bypass style)
    pthread_setname_np(pthread_self(), "xxx_Hooks");
    LOGI("entry()");
    dobby_set_near_trampoline(false);

    // Hook render ASAP - overlay must not wait on il2cpp/base.
    for (int i = 0; i < 60; i++)
    {
        void *egl = dlopen(oxorany("libEGL.so"), RTLD_NOW);
        if (egl && dlsym(egl, oxorany("eglSwapBuffers")))
        {
            init_render_hook();
            break;
        }
        sleep(1);
    }

    // If first pass missed (libEGL late), keep retrying until ImGui inits
    for (int i = 0; i < 30 && !egl_inited; i++)
    {
        init_render_hook();
        sleep(1);
    }
    LOGI("entry render: egl_inited=%d", (int)egl_inited);

    if (!_il2cpp)
        _il2cpp = new il2cpp_t();

    while (true)
    {
        if (_il2cpp->is_loaded())
        {
            base = _il2cpp->address();
            // Any valid 64-bit module base (do not require >4GB — some maps are lower)
            if (base >= 0x10000ULL)
                break;
        }
        sleep(1);
    }

    LOGI("game base ready %p - starting update::init", (void *)base);
    if (base > 0)
        c_update->init();

    // If first init raced/failed, keep retrying until sdk ready (overlay already lives)
    for (int i = 0; i < 60 && !g_sdk_ready.load(std::memory_order_acquire); ++i)
    {
        LOGI("retry update::init (%d) sdk_stage=%d", i,
             c_esp ? c_esp->dbg_sdk_stage.load(std::memory_order_relaxed) : -1);
        sleep(2);
        c_update->init();
    }

    // Keep process alive + retry egl if still dark
    for (int i = 0; i < 20 && !egl_inited; i++)
    {
        init_render_hook();
        sleep(2);
    }

    sleep(4);
    pthread_exit(nullptr);
    return nullptr;
}

// #include "zygisk/zygisk_init.hpp"
// REGISTER_ZYGISK_MODULE(tenmi_zygisk)
#include "jni.h"

// Manual openat flags - cannot include <fcntl.h> (collides with globals.hpp `bool open`).
#ifndef MELO_AT_FDCWD
#define MELO_AT_FDCWD (-100)
#define MELO_O_RDONLY 0
#define MELO_O_RDWR 2
#define MELO_O_CREAT 0100
#define MELO_O_EXCL 0200
#define MELO_O_TRUNC 01000
#endif

static int melo_openat(const char *path, int flags, int mode = 0)
{
    return (int)syscall(__NR_openat, MELO_AT_FDCWD, path, flags, mode);
}

static void melo_truncate_log()
{
    const char *paths[] = {
        "/sdcard/Download/kikaium.log",
        "/sdcard/kikaium.log",
        "/storage/emulated/0/Download/kikaium.log",
        "/storage/emulated/0/kikaium.log",
    };
    for (const char *p : paths)
    {
        int fd = melo_openat(p, MELO_O_RDWR | MELO_O_CREAT | MELO_O_TRUNC, 0666);
        if (fd >= 0)
        {
            close(fd);
            break;
        }
    }
}

static bool egl_already_owned()
{
    // With Dobby we cannot cheaply detect foreign patches — rely on once-guards.
    return false;
}

// Durable same-process once: PID file survives injector FD sweeps that kill abstract sockets.
// Returns: 1 = claimed, 0 = already held (skip), -1 = no usable path.
static int claim_via_pidfile()
{
    static const char *paths[] = {
        "/data/local/tmp/kikaium.once",
        "/sdcard/Download/kikaium.once",
        "/sdcard/kikaium.once",
        "/storage/emulated/0/Download/kikaium.once",
    };

    const pid_t me = getpid();
    bool saw_path = false;
    for (const char *p : paths)
    {
        for (int attempt = 0; attempt < 3; ++attempt)
        {
            int fd = melo_openat(p, MELO_O_RDWR | MELO_O_CREAT | MELO_O_EXCL, 0666);
            if (fd >= 0)
            {
                char buf[32];
                int n = snprintf(buf, sizeof(buf), "%d\n", (int)me);
                if (n > 0)
                    (void)write(fd, buf, (size_t)n);
                close(fd);
                LOGI("claimed process lock (pidfile %s pid=%d)", p, (int)me);
                return 1;
            }
            if (errno != EEXIST)
                break;

            saw_path = true;
            fd = melo_openat(p, MELO_O_RDONLY);
            if (fd < 0)
                break;
            char buf[64]{};
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            close(fd);
            pid_t other = (n > 0) ? (pid_t)atoi(buf) : 0;
            if (other == me)
            {
                LOGI("pidfile %s already ours (pid=%d) - skip start", p, (int)me);
                return 0;
            }
            if (other > 0 && kill(other, 0) == 0)
            {
                LOGI("pidfile %s held by live pid=%d - skip start", p, (int)other);
                return 0;
            }
            unlink(p);
            LOGI("pidfile %s stale (pid=%d) - retry", p, (int)other);
        }
    }
    return saw_path ? 0 : -1;
}

static bool claim_via_abstract_unix()
{
    int s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s < 0)
        return false;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    static const char name[] = "kikaium_once_v3";
    const size_t name_len = sizeof(name) - 1;
    addr.sun_path[0] = '\0';
    memcpy(addr.sun_path + 1, name, name_len);
    socklen_t len = static_cast<socklen_t>(offsetof(sockaddr_un, sun_path) + 1 + name_len);

    if (bind(s, reinterpret_cast<sockaddr *>(&addr), len) == 0)
    {
        LOGI("claimed process lock (abstract unix v3)");
        return true; // leak fd
    }
    const int err = errno;
    close(s);
    if (err == EADDRINUSE)
        LOGI("abstract unix v3 busy - skip start");
    return false;
}

static bool claim_process_once()
{
    // Only skip if overlay is already live in this process
    if (egl_inited)
    {
        LOGI("imgui already inited - skip duplicate start");
        return false;
    }

    const int pf = claim_via_pidfile();
    if (pf == 1)
        return true;
    // pf==0: pidfile held — still allow start if menu never came up (failed first inject)
    if (pf == 0)
        LOGI("pidfile busy but egl not inited - allow retry");

    if (claim_via_abstract_unix())
        return true;

    LOGI("locks contended - start anyway (menu recovery)");
    return true;
}

static void start_kikaium_once()
{
    static std::atomic<bool> local_started{false};
    // If first start claimed but egl never inited, allow another attempt
    if (local_started.load(std::memory_order_acquire) && egl_inited)
        return;
    if (local_started.load(std::memory_order_acquire) && !egl_inited)
    {
        LOGI("retry start_kikaium_once (egl still dark)");
        // fall through — don't double-spawn if entry thread still running
        static std::atomic<bool> retrying{false};
        bool expected = false;
        if (!retrying.compare_exchange_strong(expected, true))
            return;
        if (!claim_process_once())
        {
            retrying.store(false);
            return;
        }
        melo_truncate_log();
        LOGI("start_kikaium_once RETRY pid=%d", (int)getpid());
        std::thread(entry).detach();
        return;
    }

    bool expected = false;
    if (!local_started.compare_exchange_strong(expected, true))
        return;
    if (!claim_process_once())
    {
        local_started.store(false); // allow future retry
        return;
    }
    melo_truncate_log();
    LOGI("start_kikaium_once pid=%d", (int)getpid());
    std::thread(entry).detach();
}

// AndKitty / memfd inject often only dlopen()'s the .so - no reserved JNI key.
__attribute__((constructor)) static void kikaium_ctor()
{
    start_kikaium_once();
}

extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM *vm, void *key)
{
    (void)key; // Halalium-compat: do not require magic 1337
    JNIEnv *env = nullptr;
    if (vm)
        vm->GetEnv((void **)&env, JNI_VERSION_1_6);

    start_kikaium_once();
    return JNI_VERSION_1_6;
}