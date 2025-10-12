//
// Created by IDKTHIS on 11.10.2025.
//

#include "StackPanel.h"
#include <imgui.h>
using namespace UISystem;

StackPanel::StackPanel(const std::string& name) : UIElement(name) {}

void StackPanel::Render() {
    if (m_horizontal) {
        ImGui::BeginGroup();
        for (auto& child : m_children) {
            child->Render();
            ImGui::SameLine();
        }
        ImGui::EndGroup();
    } else {
        for (auto& child : m_children) {
            child->Render();
        }
    }
}
