#pragma once
#include "Includes/ImGui/imgui.h"
#include "imgui_internal.h"
#include <iostream>

struct page_t
{
public:
	bool *m_pVar;
	ImVec4 m_color;
	char m_szLabel[256];

	page_t() = default;
	~page_t() = default;

	page_t(bool *_var, const char *_label) noexcept
	{
		this->m_pVar = _var;
		this->m_color = {1.f, 1.f, 1.f, 0.25f};
		strcpy(this->m_szLabel, _label);
	}
};

class C_UserInterface
{
private:
	float m_fDpiScale = 1.f;
	int m_iCurrentTab = 0;

	ImFont *verdana_font;
	ImFont *verdana_font_pages;

	ImRect m_windowBounds = ImRect(ImVec2(750, 450), ImVec2(1250, 750));

	ImGuiStyle *style;

public:
	bool beginWindow(const char *name, bool *p_open = 0, ImGuiWindowFlags flags = 0);
	void endWindow();
	bool beginChild(const char *str_id, const ImVec2 &size_arg = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
	void endChild();
	bool checkbox(const char *label, bool *v);
	bool sliderscalar(const char* const& szLabel, ImGuiDataType data_type, void* pVar, const void* pMin, const void* pMax, const char* format, ImGuiSliderFlags flags);
	bool combo(const char *label, int *current_item, const char *items_separated_by_zeros);

	void init();

	void render();

	void rage();
	void visuals();
	void exploits();
	void misc();
	void config();
};