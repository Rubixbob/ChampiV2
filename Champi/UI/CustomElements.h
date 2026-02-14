#pragma once

#include "../ImGui/imgui.h"

namespace ImGui {
	bool SetCursorHover(bool result, ImGuiMouseCursor cursor_type);
	bool HandButton(const char* label, const ImVec2& size = ImVec2(0, 0));
	bool BeginHandTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0);
	bool HandSelectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
	bool HandSelectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
	bool BeginHandCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
}