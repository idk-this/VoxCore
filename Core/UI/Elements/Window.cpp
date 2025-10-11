//
// Created by IDKTHIS on 11.10.2025.
//

#include "Window.h"

#include <imgui.h>
#include <sstream>

using namespace UISystem;

Window::Window(const std::string& name) : UIElement(name) {}

void Window::Render() {
    if (ImGui::Begin(m_title.c_str(), nullptr, m_flags)) {
        for (auto& child : m_children) {
            child->Render();
        }
    }
    ImGui::End();
}

void Window::ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) {
    auto it = attributes.find("Name");
    if (it != attributes.end()) {
        m_title = it->second;
    }

    it = attributes.find("Flags");
    if (it != attributes.end()) {
        std::stringstream ss(it->second);
        std::string flag;
        while (std::getline(ss, flag, '|')) {
            if (flag == "ImGuiWindowFlags_MenuBar") m_flags |= ImGuiWindowFlags_MenuBar;
            else if (flag == "ImGuiWindowFlags_NoCollapse") m_flags |= ImGuiWindowFlags_NoCollapse;
        }
    }
}