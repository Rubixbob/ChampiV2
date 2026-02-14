#include "CustomElements.h"

bool ImGui::SetCursorHover(bool result, ImGuiMouseCursor cursor_type) {
    if (IsItemHovered()) SetMouseCursor(ImGuiMouseCursor_Hand);
    return result;
}

bool ImGui::HandButton(const char* label, const ImVec2& size_arg)
{
    return SetCursorHover(Button(label, size_arg), ImGuiMouseCursor_Hand);
}

bool ImGui::BeginHandTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
    return SetCursorHover(BeginTabItem(label, p_open, flags), ImGuiMouseCursor_Hand);
}

bool ImGui::HandSelectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg) {
    return SetCursorHover(Selectable(label, selected, flags, size_arg), ImGuiMouseCursor_Hand);
}

bool ImGui::HandSelectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg) {
    return SetCursorHover(Selectable(label, p_selected, flags, size_arg), ImGuiMouseCursor_Hand);
}

bool ImGui::BeginHandCombo(const char* label, const char* preview_value, ImGuiComboFlags flags) {
    return SetCursorHover(BeginCombo(label, preview_value, flags), ImGuiMouseCursor_Hand);
}