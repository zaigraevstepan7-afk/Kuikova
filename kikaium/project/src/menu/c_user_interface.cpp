#include "c_user_interface.hpp"

#include "includes/fonts/bold.hpp"
#include "includes/oxorany/Oxorany.hpp"
#include "globals.hpp"
#include "gui.h"
#include <cstring>

namespace kik_ui {
static constexpr ImU32 kAccent = IM_COL32(232, 168, 88, 255);
static constexpr ImU32 kAccentSoft = IM_COL32(232, 168, 88, 90);
static constexpr ImU32 kBg0 = IM_COL32(14, 15, 17, 245);
static constexpr ImU32 kBg1 = IM_COL32(20, 22, 25, 255);
static constexpr ImU32 kBg2 = IM_COL32(26, 28, 32, 255);
static constexpr ImU32 kLine = IM_COL32(48, 52, 58, 255);
static constexpr ImU32 kText = IM_COL32(236, 232, 226, 255);
static constexpr ImU32 kMuted = IM_COL32(140, 138, 132, 255);

// Display title: strip "##id" suffix from child ids.
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
} // namespace kik_ui

bool C_UserInterface::beginWindow(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    (void)p_open;
    ImGui::SetNextWindowSizeConstraints(this->m_windowBounds.Min, this->m_windowBounds.Max);
    ImGui::SetNextWindowBgAlpha(0.0f);

    if (!ImGui::Begin(name ? name : "kikaium_shell", nullptr,
                      flags | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse))
        return false;

    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    ImDrawList *dl = ImGui::GetWindowDrawList();

    // Soft vertical wash + copper hairline (not Halalium nested black frames).
    dl->AddRectFilledMultiColor(wp, ImVec2(wp.x + ws.x, wp.y + ws.y),
                                kik_ui::kBg0, kik_ui::kBg0, kik_ui::kBg1, kik_ui::kBg1);
    dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), kik_ui::kLine, 0.f, 0, 1.0f);
    dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + 3.f), kik_ui::kAccent);
    return true;
}

void C_UserInterface::endWindow()
{
    ImGui::End();
}

bool C_UserInterface::beginChild(const char *str_id, const ImVec2 &size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.10f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.21f, 0.23f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 18.f));

    if (!ImGui::BeginChild(str_id, size_arg, child_flags | ImGuiChildFlags_Borders, window_flags))
    {
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return false;
    }

    char title[64]{};
    kik_ui::panel_title(str_id, title, sizeof(title));
    if (title[0])
    {
        ImVec2 wp = ImGui::GetWindowPos();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 ts = ImGui::CalcTextSize(title);
        // Title chip sitting on the top border.
        float chip_w = ts.x + 16.f;
        ImVec2 c0(wp.x + 12.f, wp.y - ts.y * 0.5f - 2.f);
        ImVec2 c1(c0.x + chip_w, c0.y + ts.y + 4.f);
        dl->AddRectFilled(c0, c1, kik_ui::kBg2);
        dl->AddRect(c0, c1, kik_ui::kLine);
        dl->AddText(ImVec2(c0.x + 8.f, c0.y + 2.f), kik_ui::kAccent, title);
        ImGui::Dummy(ImVec2(0, 6.f));
    }
    return true;
}

void C_UserInterface::endChild()
{
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

bool C_UserInterface::checkbox(const char *label, bool *v)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const float square_sz = 18.f;
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + 8.f,
                                      pos.y + ImMax(square_sz, label_size.y) + style.FramePadding.y));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !*v;
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, ImVec2(pos.x + square_sz, pos.y + square_sz));
    ImDrawList *dl = window->DrawList;
    ImU32 frame = hovered ? IM_COL32(40, 42, 46, 255) : IM_COL32(28, 30, 34, 255);
    dl->AddRectFilled(check_bb.Min, check_bb.Max, frame, 2.f);
    dl->AddRect(check_bb.Min, check_bb.Max, hovered ? kik_ui::kAccent : kik_ui::kLine, 2.f, 0, 1.0f);
    if (*v)
    {
        dl->AddRectFilled(ImVec2(check_bb.Min.x + 3, check_bb.Min.y + 3),
                          ImVec2(check_bb.Max.x - 3, check_bb.Max.y - 3),
                          kik_ui::kAccent, 1.5f);
    }

    if (label_size.x > 0.0f)
    {
        ImVec2 lp(check_bb.Max.x + style.ItemInnerSpacing.x + 6.f,
                  check_bb.Min.y + (square_sz - label_size.y) * 0.5f);
        dl->AddText(lp, kik_ui::kText, label);
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

    extern float alpha;
    const float a = alpha > 0.02f ? alpha : 1.f;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.f, 11.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.93f, 0.91f, 0.88f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.55f, 0.38f, 0.18f, 0.45f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.70f, 0.48f, 0.22f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.90f, 0.62f, 0.28f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.11f, 0.12f, 0.14f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.15f, 0.13f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.91f, 0.66f, 0.35f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.91f, 0.66f, 0.35f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.f, 0.78f, 0.45f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.23f, 0.25f, 1.f));

    if (this->beginWindow("kikaium_shell", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        ImGui::SetWindowSize(ImVec2(1020, 640), ImGuiCond_Once);

        const float rail_w = 178.f;
        ImVec2 win = ImGui::GetWindowSize();
        ImDrawList *dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();

        // Left rail background
        dl->AddRectFilled(ImVec2(wp.x, wp.y + 3.f), ImVec2(wp.x + rail_w, wp.y + win.y), IM_COL32(16, 17, 19, 255));
        dl->AddRectFilled(ImVec2(wp.x + rail_w - 1.f, wp.y + 3.f), ImVec2(wp.x + rail_w, wp.y + win.y), IM_COL32(48, 52, 58, 255));

        ImGui::SetCursorPos(ImVec2(0, 3.f));
        ImGui::BeginChild("##kik_rail", ImVec2(rail_w, win.y - 3.f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
        {
            ImGui::Dummy(ImVec2(0, 18.f));
            ImGui::SetCursorPosX(18.f);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.90f, 0.82f, 1.f));
            ImGui::TextUnformatted("KIKAIUM");
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(18.f);
            ImGui::TextColored(ImVec4(0.70f, 0.52f, 0.28f, 1.f), "private  ·  0.39.2");
            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::SetCursorPosX(18.f);
            ImDrawList *rdl = ImGui::GetWindowDrawList();
            ImVec2 lp = ImGui::GetCursorScreenPos();
            rdl->AddRectFilled(lp, ImVec2(lp.x + rail_w - 36.f, lp.y + 1.f), IM_COL32(232, 168, 88, 160));
            ImGui::Dummy(ImVec2(0, 14.f));

            static const char *tabs[] = {"Rage", "Visuals", "Misc", "Settings", "Skins"};
            for (int i = 0; i < 5; i++)
            {
                bool selected = (m_iCurrentTab == i);
                ImGui::PushID(i);
                ImGui::SetCursorPosX(10.f);

                ImVec2 btn_sz(rail_w - 20.f, 40.f);
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                bool hovered = false;
                if (ImGui::InvisibleButton("##tabbtn", btn_sz))
                    m_iCurrentTab = i;
                hovered = ImGui::IsItemHovered();
                ImVec2 p1 = ImGui::GetItemRectMax();
                ImDrawList *tdl = ImGui::GetWindowDrawList();

                if (selected)
                {
                    tdl->AddRectFilled(p0, p1, IM_COL32(232, 168, 88, 28));
                    tdl->AddRectFilled(ImVec2(p0.x, p0.y + 6.f), ImVec2(p0.x + 3.f, p1.y - 6.f), IM_COL32(232, 168, 88, 255));
                }
                else if (hovered)
                {
                    tdl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 10));
                }

                ImVec2 ts = ImGui::CalcTextSize(tabs[i]);
                ImU32 col = selected ? IM_COL32(232, 168, 88, 255)
                                     : (hovered ? IM_COL32(220, 216, 208, 255) : IM_COL32(150, 148, 142, 255));
                tdl->AddText(ImVec2(p0.x + 18.f, p0.y + (btn_sz.y - ts.y) * 0.5f), col, tabs[i]);
                ImGui::PopID();
                ImGui::Dummy(ImVec2(0, 2.f));
            }

            ImGui::SetCursorPosY(win.y - 48.f);
            ImGui::SetCursorPosX(18.f);
            ImGui::TextColored(ImVec4(0.40f, 0.40f, 0.38f, 1.f), "tap watermark");
        }
        ImGui::EndChild();

        ImGui::SameLine(0, 0);
        ImGui::SetCursorPos(ImVec2(rail_w + 14.f, 16.f));
        ImGui::BeginChild("##kik_body", ImVec2(win.x - rail_w - 28.f, win.y - 28.f), ImGuiChildFlags_None);
        {
            switch (m_iCurrentTab)
            {
            case 0: // Rage — Halalium aim / AA surface
                visuals();
                break;
            case 1: // Visuals — ESP / chams / world
                rage();
                break;
            case 2: // Misc
                misc();
                break;
            case 3: // Settings
                config();
                break;
            case 4: // Skins
                exploits();
                break;
            }
        }
        ImGui::EndChild();
    }
    this->endWindow();

    ImGui::PopStyleColor(10);
    ImGui::PopStyleVar(6);
}

float asdasd;
int aasdd;
auto color_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder;
void C_UserInterface::rage()
{
    // Halalium Visuals tab content (ESP / chams) — Melodium widget chrome
    if (this->beginChild("##rage_left_esp", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetContentRegionAvail().y -  (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("Enable Esp"), &g.b_esp);
        checkbox(oxorany("Box"), &g.b_rect);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##boxcol"), g.m_rect, color_flags);
        checkbox(oxorany("Health Bar"), &g.b_health);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##hpcol"), g.m_health, color_flags);
        checkbox(oxorany("Bone"), &g.b_skeleton);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##bonecol"), g.m_skeleton, color_flags);
        checkbox(oxorany("name"), &g.b_name);
        checkbox(oxorany("ammo"), &g.b_ammo);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##ammocol"), g.m_ammo, color_flags);
        checkbox(oxorany("weapon"), &g.b_eweapon);
        checkbox(oxorany("Fov Check"), &g.b_fov_check);
        if (g.b_fov_check)
        {
            slider_float(oxorany("scope fov"), &g.f_fov_check, 10.f, 180.f);
            ImGui::ColorEdit4(oxorany("Fov Color"), g.m_fov_color, color_flags);
        }
        checkbox(oxorany("update matrix"), &g.update_matrix);
    }
    this->endChild();

    ImGui::SameLine();

    if (this->beginChild("##rage_right_chams", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetContentRegionAvail().y -  (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("Chams"), &g.b_players);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##enemych"), g.m_players, color_flags);
        if (g.b_players)
        {
            auto chams = g.i_players;
            ImGui::Combo(oxorany("Enemy Chams"), &chams, enemy_, IM_ARRAYSIZE(enemy_));
            g.i_players = chams;
        }
        checkbox(oxorany("Local Chams"), &g.b_local);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##localch"), g.m_local, color_flags);
        if (g.b_local)
        {
            auto dgg = g.i_local;
            ImGui::Combo(oxorany("material##local"), &dgg, local_, IM_ARRAYSIZE(local_));
            g.i_local = dgg;
        }
        checkbox(oxorany("weapom chams"), &g.weapon_chams);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##wpnch"), g.m_weapon, color_flags);
        checkbox(oxorany("tracer"), &g.b_tracer);
        checkbox(oxorany("marker"), &g.b_marker);
        checkbox(oxorany("damage"), &g.b_dmarker);
        checkbox(oxorany("solid world"), &g.b_solid);
        checkbox(oxorany("world color"), &g.b_world);
        checkbox(oxorany("fog"), &g.b_fog);
        checkbox(oxorany("sky"), &g.b_sky);
    }
    this->endChild();
}

void C_UserInterface::visuals()
{
    // Halalium Rage tab — Silent Aim / walls / AA
    static page_t hitboxes[] {
        page_t(&g.hitbox[0], oxorany("head")),
        page_t(&g.hitbox[1], oxorany("body")),
        page_t(&g.hitbox[2], oxorany("hip")),
        page_t(&g.hitbox[3], oxorany("legs"))
    };
    if (this->beginChild(oxorany("##vis_aim"), ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetContentRegionAvail().y -  (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders)) {
        checkbox(oxorany("Silent Aim"), &g.b_silent);
        checkbox(oxorany("Auto Fire"), &g.b_fire);
        checkbox(oxorany("Auto Wall"), &g.b_autowall);
        checkbox(oxorany("Through Walls"), &g.b_through_walls);
        checkbox(oxorany("Wallshot"), &g.b_wallshot);
        checkbox(oxorany("No spread"), &g.b_nospread);
        multiCombo(oxorany("hitbox"), hitboxes, IM_ARRAYSIZE(hitboxes));
        checkbox(oxorany("duck"), &g.b_duck);
        checkbox(oxorany("autostop"), &g.b_stop);
        checkbox(oxorany("Inf Ammo"), &g.b_endless);
        checkbox(oxorany("double tap"), &g.b_dt);
    }
    this->endChild();

    ImGui::SameLine();

    if (this->beginChild("##vis_aa", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetContentRegionAvail().y -  (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("Anti Aim"), &g.b_antiaim);
        auto pitch = g.i_pitch;
        ImGui::Combo(oxorany("Anti Aim Pitch"), &pitch, pitch_, IM_ARRAYSIZE(pitch_));
        g.i_pitch = pitch;
        int yaw = g.i_yaw;
        ImGui::Combo(oxorany("yaw"), &yaw, yaw_, IM_ARRAYSIZE(yaw_));
        g.i_yaw = yaw;
        checkbox(oxorany("Spin"), &g.b_spin);
        if (g.b_spin || g.b_antiaim)
            slider_float(oxorany("spin speed"), &g.f_speed, 0, 180);
        checkbox(oxorany("jitter"), &g.b_jitter);
        if (g.b_jitter)
        {
            slider_int(oxorany("range"), &g.i_range, 0, 50);
            slider_int(oxorany("frames"), &g.frames, 0, 30);
        }
        checkbox(oxorany("random in jump"), &g.b_chaos);
    }
    this->endChild();
}

void C_UserInterface::exploits()
{
    // Halalium Skins tab
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetContentRegionAvail().y -  ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("##skins_panel"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("Skin Changer"), &g.b_skin_changer);
        if (g.b_skin_changer)
        {
            slider_int(oxorany("weapon id"), &g.i_skin_weapon, 0, 64);
            slider_int(oxorany("skin id"), &g.i_skin_id, 0, 512);
            ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.50f, 1.f), "InstantiateViaServer path (Halalium)");
        }
    }

    this->endChild();
}

void C_UserInterface::misc()
{
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetContentRegionAvail().y -  ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("##misc_left"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("Third Person"), &g.b_third);
        if (g.b_third)
            slider_float(oxorany("distance"), &g.mom, 2, 3);
        checkbox(oxorany("fast walk"), &g.b_walk);
        checkbox(oxorany("auto strafer"), &g.b_strafer);
        checkbox(oxorany("remove scope"), &g.b_scope);
        slider_float(oxorany("fov"), &g.m_fov, 59.9f, 100.f);
        checkbox(oxorany("aspect ratio"), &g.b_aspect);
        if (g.b_aspect)
            slider_float(oxorany("value"), &g.f_aspect, 1, 5);
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.50f, 1.f), "Watermark always on — tap to open");
    }

    this->endChild();
}

void C_UserInterface::config()
{
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetContentRegionAvail().y -  ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("about##k"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        ImGui::TextColored(ImVec4(0.91f, 0.66f, 0.35f, 1.f), "%s", oxorany("Kikaium"));
        ImGui::TextDisabled("%s", oxorany("our build  ·  not a paste shell"));
        ImGui::Dummy(ImVec2(0, 8));
        ImGui::TextWrapped("%s", oxorany("configs in next patch"));
    }

    this->endChild();
}