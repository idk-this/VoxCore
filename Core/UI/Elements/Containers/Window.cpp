//
// Created by IDKTHIS on 11.10.2025.
//

#include "Window.h"

#include <imgui.h>
#include <sstream>

using namespace UISystem;

Window::Window(const std::string& name) : UIElement(name) {}

void Window::Render() {

    if (HasAttribute("Visible") && GetAttribute("Visible") == "false")
        return;

    std::string title = HasAttribute("Title") ? GetAttribute("Title") : "Window";

    bool canClose = !HasAttribute("CanClose") || GetAttribute("CanClose") != "false";

    ImGuiWindowFlags flags = 0;
    static const std::unordered_map<std::string, ImGuiWindowFlags> flagMap = {
        {"MenuBar", ImGuiWindowFlags_MenuBar},
        {"NoCollapse", ImGuiWindowFlags_NoCollapse},
        {"NoResize", ImGuiWindowFlags_NoResize},
        {"NoMove", ImGuiWindowFlags_NoMove},
        {"NoScrollbar", ImGuiWindowFlags_NoScrollbar},
        {"AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize}
    };

    if (HasAttribute("Flags")) {
        std::string flagStr = GetAttribute("Flags");
        std::istringstream iss(flagStr);
        std::string token;

        while (std::getline(iss, token, '|')) {
            token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
            auto it = flagMap.find(token);
            if (it != flagMap.end())
                flags |= it->second;
        }
    } else {
        flags = 0;
    }

    bool visible = true;

    if (ImGui::Begin(title.c_str(), canClose ? &visible : nullptr, flags)) {
        for (auto& child : m_children) {
            child->Render();
        }
    }
    ImGui::End();
}
