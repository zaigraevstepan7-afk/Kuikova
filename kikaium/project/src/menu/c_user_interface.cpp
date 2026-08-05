#include <cfloat>
#include "c_user_interface.hpp"

#include "includes/fonts/bold.hpp"
#include "includes/oxorany/Oxorany.hpp"
#include "includes/halalium_hooks.h"
#include "globals.hpp"
#include "gui.h"
#include <cstring>
#include <cmath>

namespace stux_ui {
// xxx — ink + amber right drawer (SO2 0.39.2)
static constexpr ImU32 kAccent = IM_COL32(255, 196, 72, 255);
static constexpr ImU32 kAccentDim = IM_COL32(255, 196, 72, 40);
static constexpr ImU32 kAccentSoft = IM_COL32(255, 196, 72, 90);
static constexpr ImU32 kBg0 = IM_COL32(6, 6, 8, 250);
static constexpr ImU32 kBg1 = IM_COL32(10, 10, 12, 255);
static constexpr ImU32 kBg2 = IM_COL32(14, 14, 16, 255);
static constexpr ImU32 kBgHeader = IM_COL32(8, 8, 10, 255);
static constexpr ImU32 kLine = IM_COL32(48, 42, 28, 255);
static constexpr ImU32 kLineSoft = IM_COL32(40, 34, 22, 180);
static constexpr ImU32 kText = IM_COL32(240, 236, 228, 255);
static constexpr ImU32 kMuted = IM_COL32(120, 110, 90, 255);

inline void panel_title(const char *str_id, char *out, size_t out_n)
{
    if (!str_id || !out_n)
        return;
    if (str_id[0] == '#' && str_id[1] == '#')
    {
        out[0] = '\0';
        return;
    }
    size_t n = 0;
    for (; str_id[n] && str_id[n] != '#' && n + 1 < out_n; ++n)
        out[n] = str_id[n];
    out[n] = '\0';
}
} // namespace stux_ui

bool C_UserInterface::beginWindow(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    (void)p_open;
    ImGui::SetNextWindowSizeConstraints(this->m_windowBounds.Min, this->m_windowBounds.Max);
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (!ImGui::Begin(name ? name : "xxx_shell", nullptr,
                      flags | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_NoNav))
        return false;

    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    ImDrawList *dl = ImGui::GetWindowDrawList();

    // Flat ink panel + mint top edge (sharp corners — not rounded copper shell)
    dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), stux_ui::kBg0, 0.f);
    dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), stux_ui::kLine, 0.f, 0, 1.0f);
    dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + 2.f), stux_ui::kAccent, 0.f);
    return true;
}

void C_UserInterface::endWindow()
{
    ImGui::End();
}

bool C_UserInterface::beginChild(const char *str_id, const ImVec2 &size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.05f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.18f, 0.10f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 14.f));

    if (!ImGui::BeginChild(str_id, size_arg, child_flags | ImGuiChildFlags_Borders, window_flags))
    {
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return false;
    }
    return true;
}

void C_UserInterface::endChild()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

bool C_UserInterface::checkbox(const char *label, bool *v)
{
    // Melodium-style: one ButtonBehavior path only (no press_edge double-toggle)
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &gctx = *GImGui;
    const ImGuiStyle &style = gctx.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = 22.f;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + 10.f,
                                      pos.y + ImMax(square_sz, label_size.y) + style.FramePadding.y + 6.f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered = false, held = false;
    const bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !*v;
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));
    ImDrawList *dl = window->DrawList;
    ImU32 frame = hovered ? IM_COL32(42, 44, 50, 255) : IM_COL32(30, 32, 36, 255);
    dl->AddRectFilled(check_bb.Min, check_bb.Max, frame, 3.f);
    dl->AddRect(check_bb.Min, check_bb.Max, hovered || *v ? stux_ui::kAccent : stux_ui::kLine, 2.f, 0, 1.0f);
    if (*v)
    {
        dl->AddRectFilled(ImVec2(check_bb.Min.x + 5, check_bb.Min.y + 5),
                          ImVec2(check_bb.Max.x - 5, check_bb.Max.y - 5),
                          stux_ui::kAccent, 1.f);
    }

    if (label_size.x > 0.0f)
    {
        ImVec2 lp(check_bb.Max.x + style.ItemInnerSpacing.x + 8.f,
                  check_bb.Min.y + (square_sz - label_size.y) * 0.5f);
        dl->AddText(lp, stux_ui::kText, label);
    }
    return pressed;
}

bool page(const char *const &szLabel, const bool &bSelected, ImVec4 &srcCol, const ImVec4 &dstCol, bool bLine, bool bVerdanaBold, ImVec2 vecSize, ImGuiSelectableFlags flags) noexcept
{
    bool ret{};
    srcCol = ImLerp(srcCol, bSelected ? dstCol : ImVec4(1.f, 1.f, 1.f, 0.25f), ImGui::GetIO().DeltaTime * 10);
    if (bSelected)
    {
        if (bLine)
        {
            const float fLineWidth = 3.f;
            const ImVec2 windowPos = ImGui::GetWindowPos();
            const ImVec2 cursorPos = ImGui::GetCursorPos();
            const ImVec2 avail = ImGui::GetContentRegionAvail();

            const ImVec2 vecPos1 = ImVec2(
                windowPos.x + cursorPos.x + avail.x - fLineWidth,
                windowPos.y + cursorPos.y);

            const ImVec2 vecPos2 = ImVec2(
                windowPos.x + cursorPos.x + avail.x,
                windowPos.y + cursorPos.y + ImGui::CalcTextSize(szLabel).y);
            ImGui::GetWindowDrawList()->AddRectFilled(vecPos1, vecPos2, (ImColor)srcCol, 100.f);
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Text, srcCol);
    ret = ImGui::Selectable(szLabel, bSelected, flags, vecSize);
    ImGui::PopStyleColor();
    if (bSelected && bVerdanaBold)
    {
       
    }
    return ret;
}

bool page(page_t *const pagee, const bool &bSelected, const ImVec4 &dstCol, bool bLine, bool bVerdanaBold, ImVec2 vecSize, ImGuiSelectableFlags flags) noexcept
{
    return page(pagee->m_szLabel, bSelected, pagee->m_color, dstCol, bLine, bVerdanaBold, vecSize, flags);
}

void endCombo() noexcept
{
    ImGui::EndCombo();
}

void multiCombo(const char *const &szLabel, page_t *const &aPages, const uint16_t &nSize) noexcept
{
    std::string sPreview = oxorany("off");

    for (uint16_t i = 0; i < nSize; ++i)
    {
        if ((*aPages[i].m_pVar))
        {
            if (sPreview != oxorany("off"))
            {
                sPreview.append(",");
                sPreview.append(aPages[i].m_szLabel);
            }
            else
            {
                sPreview = aPages[i].m_szLabel;
            }
        }
    }

    if (ImGui::BeginCombo(szLabel, sPreview.c_str()))
    {
        float aa[4] = {1.f, 1.f, 1.f, 1.f};
        for (uint8_t i = 0; i < nSize; ++i)
        {
            if (page(&aPages[i], (*aPages[i].m_pVar), ImVec4(aa[0], aa[1], aa[2], 1.f), false, true, ImVec2{}, ImGuiSelectableFlags_DontClosePopups))
            {
                (*aPages[i].m_pVar) = !(*aPages[i].m_pVar);
            }
        }
        endCombo();
    }
}

bool C_UserInterface::sliderscalar(const char* const& szLabel, ImGuiDataType data_type, void* pVar, const void* pMin, const void* pMax, const char* format, ImGuiSliderFlags flags) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(szLabel);
    const float w = ImGui::CalcItemWidth();

    const ImVec2 label_size = ImGui::CalcTextSize(szLabel, NULL, true);
    const ImVec2 cursorPos = window->DC.CursorPos;
    const ImRect frame_bb(ImVec2(cursorPos.x, cursorPos.y + label_size.y + style.FramePadding.y), ImVec2(cursorPos.x + w, cursorPos.y + label_size.y+ style.FramePadding.y + 18));
    const ImRect total_bb(cursorPos, frame_bb.Max);

    const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    ImGui::ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0);

    // Default format string when passing NULL
    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, g.LastItemData.ItemFlags);
    bool temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
    if (!temp_input_is_active)
    {
        // Tabbing or CTRL+click on Slider turns it into an input box
        const bool clicked = hovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
        const bool make_active = (clicked || g.NavActivateId == id);
        if (make_active && clicked)
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        if (make_active && temp_input_allowed)
            if ((clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                temp_input_is_active = true;

        // Store initial value (not used by main lib but available as a convenience but some mods e.g. to revert)
        if (make_active)
            memcpy(&g.ActiveIdValueOnActivation, pVar, ImGui::DataTypeGetInfo(data_type)->Size);

        if (make_active && !temp_input_is_active)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }

    if (temp_input_is_active)
    {
        // Only clamp CTRL+Click input when ImGuiSliderFlags_ClampOnInput is set (generally via ImGuiSliderFlags_AlwaysClamp)
        const bool clamp_enabled = (flags & ImGuiSliderFlags_ClampOnInput) != 0;
        return ImGui::TempInputScalar(frame_bb, id, szLabel, data_type, pVar, format, clamp_enabled ? pMin : NULL, clamp_enabled ? pMax : NULL);
    }

    // Draw frame
    const ImU32 frame_col = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::RenderNavCursor(frame_bb, id);
    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = ImGui::SliderBehavior(frame_bb, id, data_type, pVar, pMin, pMax, format, flags, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);

    // Render grab
    if (grab_bb.Max.x > grab_bb.Min.x) {
        const float pad = style.FrameBorderSize ? 1 : 0;

        float c = *reinterpret_cast<float*>(pVar);
        float cMin = *reinterpret_cast<float*>(const_cast<void*>(pMin));
        float cMax = *reinterpret_cast<float*>(const_cast<void*>(pMax));
        float frameMin = pad + pad;
        float frameMax = (frame_bb.Max.x - frame_bb.Min.x);

        int progress_ = (c - cMin) * (frameMax - frameMin) / (cMax - cMin) + frameMin;

        window->DrawList->AddRectFilled(ImVec2(frame_bb.Min.x + pad, frame_bb.Min.y + pad), ImVec2((frame_bb.Min.x + progress_) - pad, frame_bb.Max.y - pad), ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
    }

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, pVar, format);
    const ImVec2 value_label_size = ImGui::CalcTextSize(value_buf, value_buf_end, true);
    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("{", "}");
    ImGui::RenderText(ImVec2(cursorPos.x + (frame_bb.Max.x - frame_bb.Min.x) - value_label_size.x, cursorPos.y), value_buf, value_buf_end);

    if (label_size.x > 0.0f)
        ImGui::RenderText(cursorPos, szLabel);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
    return value_changed;
}

const char *FindStringByIndex(const char *buffer, int index)
{
    while (index)
    {
        while (*buffer)
            buffer++;
        buffer++;

        if (*buffer == '\0')
            break;

        index--;
    }

    return buffer;
}

int GetSubstringCount(const char *buffer)
{
    int n = 0;

    while (1)
    {
        while (*buffer)
            buffer++;
        buffer++;

        if (*buffer == '\0')
            break;

        n++;
    }

    return n + 1;
}

bool C_UserInterface::combo(const char *label, int *current_item, const char *items_separated_by_zeros)
{
    if (ImGui::BeginCombo(label, FindStringByIndex(items_separated_by_zeros, *current_item)))
    {

        for (int i = 0; i < GetSubstringCount(items_separated_by_zeros); i++)
        {

            if (ImGui::Selectable(FindStringByIndex(items_separated_by_zeros, i), i == *current_item))
                *current_item = i;
        }
        ImGui::EndCombo();
        return 1;
    }
    else
        return 0;
}

ImFont *InitFont(ImFont *verdana)
{
    ImGuiIO &io = ImGui::GetIO();

    ImFontConfig main_config;
    main_config.OversampleH = 4;
    main_config.OversampleV = 4;
    verdana = io.Fonts->AddFontFromMemoryTTF(verdana, sizeof(verdana), 12 * 4, &main_config, io.Fonts->GetGlyphRangesCyrillic());
    ImFontConfig layer_config;
    layer_config.MergeMode = true;
    layer_config.GlyphOffset.x = 0.5f;
    layer_config.GlyphOffset.y = 0.5f;
    io.Fonts->AddFontFromMemoryTTF(verdana, sizeof(verdana), 12 * 4, &layer_config, io.Fonts->GetGlyphRangesCyrillic());
    layer_config.GlyphOffset.x = -0.5f;
    layer_config.GlyphOffset.y = -0.5f;
    io.Fonts->AddFontFromMemoryTTF(verdana, sizeof(verdana), 12 * 4, &layer_config, io.Fonts->GetGlyphRangesCyrillic());

    return verdana;
}

void C_UserInterface::init()
{
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    this->style = &ImGui::GetStyle();

    ImGui::StyleColorsDark();

    this->style->WindowBorderSize = 1.f;
    this->style->ChildBorderSize = 1.f;
    this->style->FrameBorderSize = 1.f;
    this->style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    this->style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    this->style->Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    this->style->Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    this->style->Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    this->style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    this->style->Colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    this->style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    this->style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    this->style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    this->style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    this->style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    this->style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    this->style->Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    this->style->Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    this->style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    this->style->Colors[ImGuiCol_Button] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    this->style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    this->style->Colors[ImGuiCol_Header] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    this->style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    this->style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    this->style->Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    this->style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    this->style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    this->style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
    this->style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.f);
    this->style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.f);
    this->style->Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    this->style->Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    this->style->Colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    this->style->Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    this->style->Colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    this->style->Colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    this->style->Colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
    this->style->Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    this->style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    this->style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    this->style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    this->style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    this->style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    this->style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    this->style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    this->style->Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    this->style->Colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    this->style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    this->style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    this->style->Colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    this->style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    this->style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    this->style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    this->style->WindowPadding = ImVec2(20, 20);
    this->style->FramePadding = ImVec2(4, 0);
    this->style->ItemSpacing = ImVec2(15, 15);
    this->style->ScrollbarRounding = 0;
    this->style->ScrollbarSize = 20;

    this->style->AntiAliasedLines = false;
    this->style->AntiAliasedLinesUseTex = false;
    this->style->AntiAliasedFill = false;

    // InitFont(this->verdana_font);
    // InitFont(this->verdana_font_pages);

    // this->verdana_font = io.Fonts->AddFontFromMemoryTTF(zir, sizeof(zir), 12 * 4, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    // this->verdana_font_pages = io.Fonts->AddFontFromMemoryTTF(zir, sizeof(zir), 12 * 4, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    // io.Fonts->AddFontDefault();

    // verdana_font->Scale = 0.525f;
    // verdana_font_pages->Scale = 0.525f;
}

C_UserInterface a;
bool slider_int(const char *label, int *v, int v_min, int v_max)
{
    return a.sliderscalar(label, ImGuiDataType_S32, v, &v_min, &v_max, "%d", 0);
}

bool slider_float(const char *label, float *v, float v_min, float v_max)
{
    return a.sliderscalar(label, ImGuiDataType_Float, v, &v_min, &v_max, "%.3f", 0);
}

void C_UserInterface::render()
{
    if (!open)
        return;

    int lock = g_menu_input_lock.load(std::memory_order_acquire);
    if (lock > 0)
    {
        g_menu_input_lock.store(lock - 1, std::memory_order_release);
        // Don't wipe MousePos — only suppress the button that opened the menu
        ImGui::GetIO().AddMouseButtonEvent(0, false);
    }

    extern float alpha;
    const float a = (alpha < 0.02f) ? 0.02f : alpha;
    ImGui::GetStyle().TouchExtraPadding = ImVec2(10.f, 10.f);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 10.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.94f, 0.92f, 0.88f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.55f, 0.40f, 0.10f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.70f, 0.50f, 0.12f, 0.50f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.f, 0.77f, 0.28f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.09f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.12f, 0.11f, 0.09f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.77f, 0.28f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.f, 0.77f, 0.28f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.f, 0.88f, 0.50f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.18f, 0.10f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.08f, 0.09f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.12f, 0.08f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.16f, 0.06f, 1.f));

    if (c_egl && c_egl->width > 0)
    {
        const float ww = 860.f, wh = 520.f;
        ImGui::SetNextWindowPos(ImVec2((c_egl->width - ww) * 0.5f, (c_egl->heigth - wh) * 0.5f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(ww, wh), ImGuiCond_Once);
    }
    // Do NOT SetNextWindowFocus every frame — blocks watermark close clicks

    // xxx — new internal: ESP + Bypass only (SO2 0.39.2)
    if (this->beginWindow("xxx_shell", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        ImVec2 win = ImGui::GetWindowSize();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();

        const float rail_w = 120.f;
        const float rail_x = win.x - rail_w;

        dl->AddRectFilled(ImVec2(wp.x + rail_x, wp.y + 2.f), ImVec2(wp.x + win.x, wp.y + win.y), stux_ui::kBgHeader);
        dl->AddRectFilled(ImVec2(wp.x + rail_x, wp.y + 2.f), ImVec2(wp.x + rail_x + 2.f, wp.y + win.y), stux_ui::kAccent);

        dl->AddRectFilled(ImVec2(wp.x, wp.y + 2.f), ImVec2(wp.x + rail_x, wp.y + 44.f), stux_ui::kBg1);
        dl->AddText(ImVec2(wp.x + 16.f, wp.y + 14.f), stux_ui::kAccent, "xxx");
        dl->AddText(ImVec2(wp.x + 48.f, wp.y + 16.f), stux_ui::kMuted, "0.39.2");

        // Close button in header (always works even if watermark is blocked)
        {
            ImGui::SetCursorPos(ImVec2(rail_x - 72.f, 8.f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.10f, 0.08f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.20f, 0.12f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.25f, 0.12f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.94f, 0.92f, 0.88f, 1.f));
            if (ImGui::Button("CLOSE", ImVec2(64.f, 28.f)))
                open = false;
            ImGui::PopStyleColor(4);
        }

        static const char *tabs[] = {"01 ESP", "02 BYP"};
        ImGui::SetCursorPos(ImVec2(rail_x + 8.f, 56.f));
        for (int i = 0; i < 2; i++)
        {
            ImGui::PushID(i);
            ImGui::SetCursorPosX(rail_x + 8.f);
            bool selected = (m_iCurrentTab == i);
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImGui::PushStyleColor(ImGuiCol_Button, selected ? ImVec4(1.f, 0.77f, 0.28f, 0.18f) : ImVec4(0.06f, 0.06f, 0.07f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 0.77f, 0.28f, 0.28f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 0.77f, 0.28f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_Text, selected ? ImVec4(1.f, 0.77f, 0.28f, 1.f) : ImVec4(0.70f, 0.66f, 0.58f, 1.f));
            if (ImGui::Button(tabs[i], ImVec2(rail_w - 16.f, 52.f)))
                m_iCurrentTab = i;
            ImGui::PopStyleColor(4);

            if (selected)
                dl->AddRectFilled(ImVec2(wp.x + rail_x, p0.y + 10.f), ImVec2(wp.x + rail_x + 2.f, p0.y + 42.f), stux_ui::kAccent);
            ImGui::Dummy(ImVec2(0, 6.f));
            ImGui::PopID();
        }

        ImGui::SetCursorPos(ImVec2(14.f, 52.f));
        ImGui::BeginChild("##xxx_body", ImVec2(rail_x - 28.f, win.y - 66.f), ImGuiChildFlags_None);
        {
            switch (m_iCurrentTab)
            {
            case 0: rage(); break;
            case 1: config(); break;
            default: m_iCurrentTab = 0; rage(); break;
            }
        }
        ImGui::EndChild();
    }
    this->endWindow();

    ImGui::PopStyleColor(13);
    ImGui::PopStyleVar(6);
}

float asdasd;
int aasdd;
auto color_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder;

static void stux_section(const char *title)
{
    ImGui::Dummy(ImVec2(0, 2.f));
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddText(p, stux_ui::kAccent, title);
    ImVec2 ts = ImGui::CalcTextSize(title);
    ImGui::Dummy(ImVec2(0, ts.y + 2.f));
    p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(p, ImVec2(p.x + 36.f, p.y + 2.f), stux_ui::kAccentSoft);
    ImGui::Dummy(ImVec2(0, 10.f));
}

static void stux_two_col(float &left, float &right, float &h)
{
    const float gap = 12.f;
    const float avail_x = ImGui::GetContentRegionAvail().x;
    const float avail_y = ImGui::GetContentRegionAvail().y;
    left = (avail_x - gap) * 0.52f;
    right = avail_x - gap - left;
    h = avail_y;
    if (left < 160.f) { left = avail_x; right = 0.f; }
}

void C_UserInterface::visuals()
{
    // unused — xxx ships ESP+Bypass only
}

void C_UserInterface::rage()
{
    // Halalium Visuals ESP only (Lemming) — no Melodium Name/Weapon/Ammo/Skeleton
    float L = 0.f, R = 0.f, h = 0.f;
    stux_two_col(L, R, h);
    if (this->beginChild("##xxx_esp_a", ImVec2(L, h), ImGuiChildFlags_Borders))
    {
        stux_section("ESP");
        checkbox(oxorany("Enable Esp"), &g.b_esp);
        checkbox(oxorany("Box"), &g.b_rect);
        {
            int bt = g.i_box_type;
            ImGui::Combo(oxorany("Box Type"), &bt, "Full\0Corner\0Rounded\0\0");
            g.i_box_type = bt;
        }
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("Box Color"), g.m_rect, color_flags);
        slider_float(oxorany("Thickness"), &g.f_box_thickness, 0.5f, 5.f);
        if (g.i_box_type == 1)
            slider_float(oxorany("Corner Size"), &g.f_corner_size, 0.05f, 0.5f);
        checkbox(oxorany("Outline"), &g.b_box_outline);
        checkbox(oxorany("Fill"), &g.b_box_fill);
        if (g.b_box_fill)
            slider_float(oxorany("Fill Alpha"), &g.f_box_fill_alpha, 0.f, 1.f);
        checkbox(oxorany("Health Bar"), &g.b_health);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##hpcol"), g.m_health, color_flags);
        checkbox(oxorany("Distance"), &g.b_distance);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("Distance Color"), g.m_distance, color_flags);
    }
    this->endChild();
    if (R > 40.f)
    {
        ImGui::SameLine(0, 12.f);
        if (this->beginChild("##xxx_esp_b", ImVec2(R, h), ImGuiChildFlags_Borders))
        {
            stux_section("VISUALS");
            checkbox(oxorany("Through Walls"), &g.b_through_walls);
            ImGui::Dummy(ImVec2(0, 8.f));
            ImGui::TextDisabled("Halalium / Lemming ESP");
            ImGui::TextDisabled("no Melodium name/ammo");
        }
        this->endChild();
    }
}

void C_UserInterface::misc()
{
}

void C_UserInterface::exploits()
{
}

void C_UserInterface::config()
{
    // xxx tab 02 — Bypass
    float w = ImGui::GetContentRegionAvail().x;
    float h = ImGui::GetContentRegionAvail().y;
    if (this->beginChild(oxorany("##xxx_byp"), ImVec2(w, h), ImGuiChildFlags_Borders))
    {
        stux_section("BYPASS");
        bool prev = g.b_bypass;
        checkbox(oxorany("Getrr Bypass"), &g.b_bypass);
        if (g.b_bypass != prev || g.b_bypass != hhooks::getrr_is_armed())
        {
            if (g.b_bypass)
            {
                if (unity_base)
                    hhooks::install_getrr_bypass(unity_base);
            }
            else
            {
                hhooks::uninstall_getrr_bypass();
            }
        }
        ImGui::TextDisabled("%s", hhooks::getrr_is_armed()
                                      ? oxorany("status: ARMED")
                                      : oxorany("status: OFF"));
        ImGui::Dummy(ImVec2(0, 12.f));
        stux_section("OVERLAY");
        bool wm = g.b_watermark;
        checkbox(oxorany("Watermark"), &g.b_watermark);
        // Don't allow hiding watermark while menu is open (soft-lock)
        if (open && !g.b_watermark)
            g.b_watermark = true;
        (void)wm;
        ImGui::Dummy(ImVec2(0, 8.f));
        ImGui::TextColored(ImVec4(1.f, 0.77f, 0.28f, 1.f), "xxx");
        ImGui::TextColored(ImVec4(0.55f, 0.50f, 0.40f, 1.f), "standoff 2 0.39.2");
        ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.40f, 1.f), "andkitty memfd inject");
    }
    this->endChild();
}
