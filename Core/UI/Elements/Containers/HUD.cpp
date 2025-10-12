//
// Created by IDKTHIS on 11.10.2025.
//

#include "HUD.h"
#include <imgui.h>
#include <sstream>

using namespace UISystem;

HUD::HUD(const std::string& name)
    : UIElement(name)
{
}

void HUD::Render()
{
    if (HasAttribute("Visible") && GetAttribute("Visible") == "false")
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_NoSavedSettings;

    // Позволяем переопределять через XML
    if (HasAttribute("Flags")) {
        std::string flagStr = GetAttribute("Flags");
        std::istringstream iss(flagStr);
        std::string token;

        static const std::unordered_map<std::string, ImGuiWindowFlags> flagMap = {
            {"NoBackground", ImGuiWindowFlags_NoBackground},
            {"NoInputs", ImGuiWindowFlags_NoInputs},
            {"AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize}
        };

        while (std::getline(iss, token, '|')) {
            token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
            auto it = flagMap.find(token);
            if (it != flagMap.end())
                flags |= it->second;
        }
    }

    ImVec2 pos = ImVec2(0, 0);
    ImVec2 size = ImGui::GetIO().DisplaySize;

    if (HasAttribute("X") && HasAttribute("Y"))
        pos = ImVec2(std::stof(GetAttribute("X")), std::stof(GetAttribute("Y")));

    if (HasAttribute("Width") && HasAttribute("Height"))
        size = ImVec2(std::stof(GetAttribute("Width")), std::stof(GetAttribute("Height")));

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    if (ImGui::Begin(("##HUD_" + m_name).c_str(), nullptr, flags))
    {
        for (auto& child : m_children)
            child->Render();
    }
    ImGui::End();
}
