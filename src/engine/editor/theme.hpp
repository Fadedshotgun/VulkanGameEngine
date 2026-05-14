#include "imgui.h"

namespace editor
{
    void applyTheme()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        style.TabRounding = 0;
        style.ItemSpacing = ImVec2(4, 6);

        ImVec4 *colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 0.50f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.53f, 0.53f, 0.53f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.80f);
        colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.06f, 0.06f, 0.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_TabDimmed] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.48f, 0.00f, 0.00f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.48f, 0.11f, 0.11f, 0.70f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.00f, 0.00f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.67f);
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.00f, 0.00f, 0.40f);
        colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.00f, 0.00f, 0.67f);
    }
}