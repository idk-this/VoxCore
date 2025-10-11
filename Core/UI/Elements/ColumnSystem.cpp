//
// Created by IDKTHIS on 11.10.2025.
//

#include "ColumnSystem.h"
#include <imgui.h>
#include <sstream>

using namespace UISystem;

ColumnSystem::ColumnSystem(const std::string& name) : UIElement(name) {}

    void ColumnSystem::Render() {
        ImGui::BeginGroup();

        float availableWidth = ImGui::GetContentRegionAvail().x;
        float columnWidth = availableWidth / m_columns;

        int currentColumn = 0;
        bool isNewLine = true;

        for (auto& child : m_children) {
            int columnSpan = std::stoi(child->GetAttribute("ColumnSpan"));

            if (currentColumn + columnSpan > m_columns) {
                currentColumn = 0;
                isNewLine = true;
                ImGui::NewLine();
            }

            if (!isNewLine && currentColumn > 0) {
                ImGui::SameLine();
            }

            float elementWidth = columnWidth * columnSpan;

            ImGui::BeginGroup();
            ImGui::PushItemWidth(elementWidth);
            child->Render();
            ImGui::PopItemWidth();
            ImGui::EndGroup();

            currentColumn += columnSpan;
            isNewLine = false;

            if (currentColumn >= m_columns) {
                currentColumn = 0;
                isNewLine = true;
            }
        }

        ImGui::EndGroup();
    }

    void ColumnSystem::ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) {
        if (HasAttribute("Columns")) {
            m_columns = std::stoi(GetAttribute("Columns"));
        }
        if (HasAttribute("ColumnSizes")) {
            std::stringstream ss(GetAttribute("ColumnSizes"));
            std::string size;
            while (std::getline(ss, size, ',')) {
                m_columnSizes.push_back(std::stoi(size));
            }
        }
    }

    void ColumnSystem::AddChild(std::shared_ptr<UIElement> child)
    {if (child) {
            child->SetParent(this);
            m_children.push_back(child);
            auto columnSpanAttr = child->GetAttribute("ColumnSpan");
            if (!columnSpanAttr.empty()) {
                m_childColumnSpans[child->GetName()] = std::stoi(columnSpanAttr);
            }
        }
    }