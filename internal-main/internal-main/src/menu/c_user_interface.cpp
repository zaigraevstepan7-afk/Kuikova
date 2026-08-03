#include "c_user_interface.hpp"

#include "includes/fonts/bold.hpp"
#include "includes/oxorany/Oxorany.hpp"
#include "globals.hpp"
#include "gui.h"

bool test0 = false;

bool C_UserInterface::beginWindow(const char *name, bool *p_open, ImGuiWindowFlags flags)
{

    ImGui::SetNextWindowSizeConstraints(this->m_windowBounds.Min, this->m_windowBounds.Max);

    if (ImGui::Begin("picadff", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
    {
        for (int i = 0; i < 8; i++)
        {

            ImColor borderCol = ImColor(0, 0, 0, 255);

            if (i == 1 || i == 8 - 1)
                borderCol = ImColor(55, 55, 55, 255);
            else if (i == 0)
                borderCol = ImColor(0, 0, 0, 255);
            else
                borderCol = ImColor(35, 35, 35, 255);

            ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + i, ImGui::GetWindowPos().y + i), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - i, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - i), borderCol);
        }
        return 1;
    }
    else
        return 0;
}

void C_UserInterface::endWindow()
{
    ImGui::End();
}

bool C_UserInterface::beginChild(const char *str_id, const ImVec2 &size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags)
{
    if (ImGui::BeginChild(str_id, size_arg, child_flags, window_flags))
    {
        const char *labeltext = str_id;
        ImVec2 labelsize = ImGui::CalcTextSize(labeltext);
        ImVec2 labelpos = ImVec2(ImGui::GetWindowPos().x + 15, ImGui::GetWindowPos().y - (labelsize.y / 2));

        ImGui::GetWindowDrawList()->AddRect(ImGui::GetWindowPos(), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y), ImColor(0, 0, 0, 255));
        ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 1, ImGui::GetWindowPos().y + 1), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 1, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 1), ImColor(40, 40, 40, 255));
        ImGui::GetWindowDrawList()->AddRect(ImVec2(ImGui::GetWindowPos().x + 2, ImGui::GetWindowPos().y + 2), ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 2, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y - 2), ImColor(25, 25, 25, 255));

        ImGui::GetForegroundDrawList()->AddText(labelpos, ImColor(255, 255, 255, 255), labeltext);
        return 1;
    }
    else
        return 0;
}

void C_UserInterface::endChild()
{
    ImGui::EndChild();
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

    const float square_sz = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, ImVec2(pos.x + square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + 10, pos.y + label_size.y + style.FramePadding.y * 2.0f));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    const bool is_visible = ImGui::ItemAdd(total_bb, id);
    const bool is_multi_select = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
    if (!is_visible)
        if (!is_multi_select || !g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(total_bb)) // Extra layer of "no logic clip" for box-select support
        {
            IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
            return false;
        }

    // Range-Selection/Multi-selection support (header)
    bool checked = *v;
    if (is_multi_select)
        ImGui::MultiSelectItemHeader(id, &checked, NULL);

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

    // Range-Selection/Multi-selection support (footer)
    if (is_multi_select)
        ImGui::MultiSelectItemFooter(id, &checked, &pressed);
    else if (pressed)
        checked = !checked;

    if (*v != checked)
    {
        *v = checked;
        pressed = true; // return value
        ImGui::MarkItemEdited(id);
    }

    const ImRect check_bb(pos, ImVec2(square_sz + pos.x, square_sz + pos.y));
    if (is_visible)
    {
        ImGui::RenderNavCursor(total_bb, id);
        // ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
        window->DrawList->AddRectFilledMultiColor(check_bb.Min, check_bb.Max, ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255), ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered
                                                                                                                                                                                                    : ImGuiCol_FrameBg),
                                                  ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered
                                                                                                                          : ImGuiCol_FrameBg));
        ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
        if (*v)
        {
            const float pad = ImMax(1.0f, IM_TRUNC(square_sz / 6.0f));
            window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x + 1, check_bb.Min.y + 1), ImVec2(check_bb.Max.x - 1, check_bb.Max.y - 1), check_col, style.FrameRounding);
        }
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, ImGui::GetColorU32(ImGuiCol_Border));
    }
    const ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x + 10, check_bb.Min.y + style.FramePadding.y);
    if (g.LogEnabled)
        ImGui::LogRenderedText(&label_pos, *v ? "[x]" : "[ ]");
    if (is_visible && label_size.x > 0.0f)
        ImGui::RenderText(label_pos, label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
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
    if (open)
    {
        // ImGui::PushFont(verdana_font);

        if (this->beginWindow("picadff", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::SetWindowSize(ImVec2(1250, 750), ImGuiCond_Once);

            if (this->beginChild(" ", ImVec2(ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2), 48), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                const char *labeltext = ("t.me/melrele");
                ImVec2 labelsize = ImGui::CalcTextSize(labeltext);
                ImVec2 labelpos = ImVec2((ImGui::GetWindowPos().x + (ImGui::GetWindowSize().x / 2)) - (labelsize.x / 2), (ImGui::GetWindowPos().y + (ImGui::GetWindowSize().y / 2)) - (labelsize.y / 2));
                ImGui::GetWindowDrawList()->AddText(labelpos, ImColor(255, 255, 255, 255), labeltext);
            }
            this->endChild();

            switch (m_iCurrentTab)
            {
            case 0:
                rage();
                break;
            case 1:
                visuals();
                break;
            case 2:
                exploits();
                break;
            case 3:
                misc();
                break;
            case 4:
                config();
                break;
            }

            if (this->beginChild("  ", ImVec2(ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2), 65), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
            {
                for (int i = 0; i < 5; i++)
                {
                    char tabName[32];
                    switch (i)
                    {
                    case 0:
                        snprintf(tabName, sizeof(tabName), oxorany("visuals##%i"), i);
                        break;
                    case 1:
                        snprintf(tabName, sizeof(tabName), oxorany("ragebot##%i"), i);
                        break;
                    case 2:
                        snprintf(tabName, sizeof(tabName), oxorany("exploits##%i"), i);
                        break;
                    case 3:
                        snprintf(tabName, sizeof(tabName), oxorany("misc##%i"), i);
                        break;
                    case 4:
                        snprintf(tabName, sizeof(tabName), oxorany("config##%i"), i);
                        break;
                    }

                    // ImGui::PopFont();
                    // ImGui::PushFont(verdana_font_pages);

                    if (m_iCurrentTab == i)
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    else
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));

                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

                    if (ImGui::Selectable(tabName, m_iCurrentTab == i, 0, ImVec2((ImGui::GetWindowSize().x / 5) - (ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().WindowPadding.x), 0)))
                        m_iCurrentTab = i;

                    ImGui::PopStyleVar(1);

                    ImGui::PopStyleColor();

                    // ImGui::PopFont();

                    ImGui::SameLine();
                }
            }
            this->endChild();
        }
        this->endWindow();
    }
}

float asdasd;
int aasdd;
auto color_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder;
void C_UserInterface::rage()
{
    if (this->beginChild("esp", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetWindowSize().y - 112 - (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("update matrix"), &g.update_matrix);
        checkbox(oxorany("esp"), &g.b_esp);
        checkbox(oxorany("box"), &g.b_rect);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dsfssfsfs"), g.m_rect, color_flags);
        checkbox(oxorany("name"), &g.b_name);
        checkbox(oxorany("health"), &g.b_health);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dfsdfgfdsdff"), g.m_health, color_flags);
        checkbox(oxorany("ammo"), &g.b_ammo);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##fgdfgdfg"), g.m_ammo, color_flags);
        checkbox(oxorany("skeleton"), &g.b_skeleton);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##fguujhdfg"), g.m_skeleton, color_flags);
        checkbox(oxorany("weapon"), &g.b_eweapon);
    }
    this->endChild();

    ImGui::SameLine();

    if (this->beginChild("visuals", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetWindowSize().y - 112 - (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        ImGui::Text(oxorany("world"));
        checkbox(oxorany("solid world"), &g.b_solid);
        checkbox(oxorany("world color"), &g.b_world);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##daaaaaa"), g.m_world, color_flags);
        checkbox(oxorany("fog"), &g.b_fog);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##sdfgdsfgdsg"), g.m_fog, color_flags);
        if (g.b_fog)
        {
            slider_float(oxorany("fog start"), &g.f_start, 1.0f, 6.0f);
            slider_float(oxorany("fog end"), &g.f_end, 1.0f, 50.0f);
        }
        checkbox(oxorany("sky"), &g.b_sky);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##sdfgdfdgsfgdsg"), g.m_sky, color_flags);
        ImGui::Text(oxorany("chams"));
        checkbox(oxorany("players"), &g.b_players);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dfgdfg"), g.m_players, color_flags);

        if (g.b_players)
        {
            auto chams = g.i_players;
            ImGui::Combo(oxorany("material##DDsdfDd"), &chams, enemy_, IM_ARRAYSIZE(enemy_));
            g.i_players = chams;
        }
        checkbox(oxorany("local"), &g.b_local);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dsgghhgsd"), g.m_local, color_flags);
        if (g.b_local)
        {
            auto dgg = g.i_local;
            ImGui::Combo(oxorany("material##DDddhD"), &dgg, local_, IM_ARRAYSIZE(local_));
            g.i_local = dgg;
        }
        checkbox(oxorany("weapom chams"), &g.weapon_chams);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dsfsdfsdfdsfsfdsfjhjghjgh"), g.m_weapon, color_flags);
        if (g.weapon_chams)
        {
            auto dgg = g.i_weapon;
            ImGui::Combo(oxorany("material##DDdghjhgjghjgdhD"), &dgg, hit_, IM_ARRAYSIZE(hit_));
            g.i_weapon = dgg;
        }
        ImGui::Text(oxorany("bullet"));
        checkbox(oxorany("tracer"), &g.b_tracer);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##fggghfjh"), g.m_tracer, color_flags);
        checkbox(oxorany("marker"), &g.b_marker);
        ImGui::SameLine();
        ImGui::ColorEdit4(oxorany("##dfgfdg"), g.m_marker, color_flags);
        checkbox(oxorany("damage"), &g.b_dmarker);
    }
    this->endChild();
}

void C_UserInterface::visuals()
{
    static page_t hitboxes[] {
        page_t(&g.hitbox[0], oxorany("head")),
        page_t(&g.hitbox[1], oxorany("body")),
        page_t(&g.hitbox[2], oxorany("hip")),
        page_t(&g.hitbox[3], oxorany("legs"))
    };
    if (this->beginChild(oxorany("silent"), ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetWindowSize().y - 112 - (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders)) {
        checkbox(oxorany("silent"), &g.b_silent);
        checkbox(oxorany("auto fire"), &g.b_fire);
        multiCombo(oxorany("hitbox"), hitboxes, IM_ARRAYSIZE(hitboxes));
        checkbox(oxorany("duck"), &g.b_duck);
        checkbox(oxorany("autostop"), &g.b_stop);
        checkbox(oxorany("endless ammo"), &g.b_endless);
        checkbox(oxorany("double tap"), &g.b_dt);

        // checkbox(oxorany("predict"), &g.predict);
        // if (g.predict)
        //     slider_float(oxorany("factor"), &g.factor, 0, 10);
    }
    this->endChild();

    ImGui::SameLine();

    if (this->beginChild("anti aims", ImVec2((ImGui::GetWindowSize().x - (ImGui::GetStyle().WindowPadding.x * 2 + ImGui::GetStyle().ItemSpacing.x)) / 2, ImGui::GetWindowSize().y - 112 - (ImGui::GetStyle().WindowPadding.y * 2 + ImGui::GetStyle().ItemSpacing.y * 2)), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("anti aims"), &g.b_antiaim);
        auto pitch = g.i_pitch;
        ImGui::Combo(oxorany("pitch"), &pitch, pitch_, IM_ARRAYSIZE(pitch_));
        g.i_pitch = pitch;
        int yaw = g.i_yaw;
        ImGui::Combo(oxorany("yaw"), &yaw, yaw_, IM_ARRAYSIZE(yaw_));
        g.i_yaw = yaw;
        checkbox(oxorany("jitter"), &g.b_jitter);
        if (g.b_jitter)
        {
            slider_int(oxorany("range"), &g.i_range, 0, 50);
            slider_int(oxorany("frames"), &g.frames, 0, 30);

        }
        slider_float(oxorany("spin speed"), &g.f_speed, 0, 180);
        checkbox(oxorany("random in jump"), &g.b_chaos);
    }
    this->endChild();
}

void C_UserInterface::exploits()
{
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetWindowSize().y - 112 - ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("exploits"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        // checkbox(oxorany("one hit kill"), &g.b_onehit);
        // checkbox(oxorany("fire rate"), &g.b_frate);
        // checkbox(oxorany("god mode"), &g.b_god);
        // checkbox(oxorany("set health"), &g.sethp);
        // slider_float(oxorany("dur"), &g.dur, 1, 1000);
    }

    this->endChild();
}

void C_UserInterface::misc()
{
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetWindowSize().y - 112 - ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("misc"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        checkbox(oxorany("fast walk"), &g.b_walk);
        checkbox(oxorany("auto strafer"), &g.b_strafer);
        checkbox(oxorany("third person"), &g.b_third);
        if (g.b_third)
        slider_float(oxorany("distance"), &g.mom, 2, 3);
        checkbox(oxorany("remove scope"), &g.b_scope);
        slider_float(oxorany("fov"), &g.m_fov, 59.9f, 100.f);
        checkbox(oxorany("aspect ratio"), &g.b_aspect);
        if (g.b_aspect) {
         slider_float(oxorany("value"), &g.f_aspect, 1, 5);
        }
    }

    this->endChild();
}

void C_UserInterface::config()
{
    float childWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2;
    float childHeight = ImGui::GetWindowSize().y - 112 - ImGui::GetStyle().WindowPadding.y * 2 - ImGui::GetStyle().ItemSpacing.y * 2;

    if (this->beginChild(oxorany("configs"), ImVec2(childWidth, childHeight), ImGuiChildFlags_Borders))
    {
        ImGui::Text(oxorany("t.me/melrele"));
        ImGui::Text(oxorany("configs in next patch maybe"));
    }

    this->endChild();
}