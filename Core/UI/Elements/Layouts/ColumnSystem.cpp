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

    ImVec2 posStart = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float availableWidth = avail.x;
    float availableHeight = avail.y;

    if (m_columns <= 0)
        m_columns = 1;

    float totalWidth = availableWidth;
    if (HasAttribute("Width")) {
        try {
            totalWidth = std::stof(GetAttribute("Width"));
            totalWidth = std::min(totalWidth, availableWidth); // Не превышаем доступную ширину
        }
        catch (...) {
            totalWidth = availableWidth;
        }
    }

    float columnWidth = totalWidth / static_cast<float>(m_columns);

    float startX = posStart.x;
    if (HasAttribute("Align")) {
        std::string align = GetAttribute("Align");
        if (align == "Center")
            startX = posStart.x + (availableWidth - totalWidth) * 0.5f;
        else if (align == "Right")
            startX = posStart.x + (availableWidth - totalWidth);
    }

    float startY = posStart.y;
    if (HasAttribute("VerticalAlign")) {
        std::string valign = GetAttribute("VerticalAlign");
        if (valign == "Bottom")
            startY = posStart.y + availableHeight;
    }

    int currentColumn = 0;
    float currentY = startY;

    for (auto& child : m_children) {
        int columnSpan = 1;
        if (child->HasAttribute("ColumnSpan")) {
            try { columnSpan = std::stoi(child->GetAttribute("ColumnSpan")); }
            catch (...) { columnSpan = 1; }
            if (columnSpan < 1) columnSpan = 1;
            if (columnSpan > m_columns) columnSpan = m_columns;
        }

        if (currentColumn + columnSpan > m_columns) {
            currentColumn = 0;
            currentY += ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.y;
        }

        float elementWidth = columnWidth * columnSpan;
        float columnStartX = startX + columnWidth * currentColumn;
        float childX = columnStartX;

        float childWidth = elementWidth;
        if (child->HasAttribute("Width")) {
            try {
                childWidth = std::stof(child->GetAttribute("Width"));
                childWidth = std::min(childWidth, elementWidth);
            }
            catch (...) {}
        }

        if (child->HasAttribute("Align")) {
            std::string align = child->GetAttribute("Align");
            if (align == "Center")
                childX = columnStartX + (elementWidth - childWidth) * 0.5f;
            else if (align == "Right")
                childX = columnStartX + (elementWidth - childWidth);
        }
        ImVec4 BGColor = ImVec4(0, 0, 0, 0);
        if (HasAttribute("BackgroundColor"))
        {
            BGColor = ParseColor(GetAttribute("BackgroundColor"));
        }
        ImGui::SetCursorScreenPos(ImVec2(childX, currentY));

        ImGui::PushItemWidth(childWidth);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, BGColor);
        ImGui::BeginChild((child->GetName() + "_container").c_str(),
                         ImVec2(childWidth, 0),
                         ImGuiChildFlags_AutoResizeY);
        ImGui::PopStyleColor();

        child->Render();

        ImGui::EndChild();
        ImGui::PopItemWidth();

        currentColumn += columnSpan;
        if (currentColumn >= m_columns) {
            currentColumn = 0;
            float itemHeight = ImGui::GetItemRectSize().y;
            currentY += itemHeight + ImGui::GetStyle().ItemSpacing.y;
        }
    }

    ImGui::EndGroup();
}


void ColumnSystem::SetDataContext(DataContext* dataContext)
{
    UIElement::SetDataContext(dataContext);
    if (HasAttribute("Columns")) {
        try {
            m_columns = std::stoi(GetAttribute("Columns"));
            if (m_columns < 1) m_columns = 1;
        } catch (...) {
            m_columns = 1;
        }
    }
}

void ColumnSystem::AddChild(std::shared_ptr<UIElement> child) {
    if (child) {
        child->SetParent(this);
        m_children.push_back(child);
    }
}
