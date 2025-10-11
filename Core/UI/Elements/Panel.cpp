//
// Created by IDKTHIS on 11.10.2025.
//

#include "Panel.h"
#include <imgui.h>

using namespace UISystem;

Panel::Panel(const std::string& name) : UIElement(name) {}

void Panel::Render() {
    ImGui::BeginGroup();
    for (auto& child : m_children) {
        child->Render();
    }
    ImGui::EndGroup();
}

void Panel::ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) {
    auto it = attributes.find("Width");
    if (it != attributes.end()) {
        ParseSize(it->second, m_width);
    }

    it = attributes.find("Height");
    if (it != attributes.end()) {
        ParseSize(it->second, m_height);
    }
}