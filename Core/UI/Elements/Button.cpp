//
// Created by IDKTHIS on 11.10.2025.
//

#include "Button.h"

#include <cmath>
#include <imgui.h>
#include <sstream>

using namespace UISystem;

Button::Button(const std::string& name) : UIElement(name) {}

// --- Вспомогательная функция для парсинга "prop=Width;to=40;dur=0.7"
static UIAnimation ParseAnimationString(const std::string& str) {
    UIAnimation anim;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ';')) {
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;

        std::string key = token.substr(0, eq);
        std::string val = token.substr(eq + 1);

        if (key == "prop") anim.property = val;
        else if (key == "to") anim.toValue = std::stof(val);
        else if (key == "dur") anim.duration = std::stof(val);
    }

    if (!anim.property.empty())
        anim.active = true;

    return anim;
}

void Button::Render() {
    float deltaTime = ImGui::GetIO().DeltaTime;

    // --- Обработка анимаций ---
    for (auto& [name, anim] : m_activeAnimations) {
        if (!anim.active) continue;
        float diff = anim.toValue - anim.current;
        if (std::fabs(diff) > 0.01f) {
            float step = diff * (deltaTime / anim.duration);
            anim.current += step;
        } else {
            anim.current = anim.toValue;
            anim.active = false;
        }

        if (anim.property == "Width") m_width = anim.current;
        else if (anim.property == "Height") m_height = anim.current;
        else if (anim.property == "ColumnSpan") m_attributes["ColumnSpan"] = std::to_string((int)anim.current);
    }

    // --- Пространство, выделенное колонкой ---
    float availableWidth = m_width.value; // от ColumnSystem
    if (availableWidth <= 0)
        availableWidth = ImGui::GetContentRegionAvail().x;

    // --- Фактический размер кнопки ---
    float buttonWidth = availableWidth; // по умолчанию растягиваем
    if (HasAttribute("Width")) {
        try {
            buttonWidth = std::stof(GetAttribute("Width")); // фиксированный размер
            if (buttonWidth > availableWidth)
                buttonWidth = availableWidth; // ограничение
        } catch (...) { /* игнорировать */ }
    }

    float buttonHeight = ImGui::GetFrameHeight();
    if (HasAttribute("Height")) {
        try {
            buttonHeight = std::stof(GetAttribute("Height"));
        } catch (...) { /* игнорировать */ }
    }
    if (HasAttribute("Align")) {
        std::string align = GetAttribute("Align");
        float offset = 0.0f;

        if (align == "Center") {
            offset = (availableWidth - buttonWidth) / 2.0f;
        } else if (align == "Right") {
            offset = availableWidth - buttonWidth;
        }

        if (offset > 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    std::string buttonText = m_name;
    for (auto& child : m_children) {
        if (child->GetType() == "TextBlock" && child->HasAttribute("Text")) {
            buttonText = child->GetAttribute("Text");
            break;
        }
    }
    std::string tooltip;
    if (HasAttribute("Tooltip"))
        tooltip = GetAttribute("Tooltip");
    if (ImGui::Button(buttonText.c_str(), ImVec2(buttonWidth, buttonHeight))) {
        if (OnClick) OnClick();
        if (!m_clickHandler.empty() && m_dataContext) {
            auto* dataContext = static_cast<SimpleDataContext*>(m_dataContext);
            auto handler = dataContext->GetEvent(m_clickHandler);
            if (handler) handler();
        }
    }

    if (!tooltip.empty() && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip.c_str());
    for (auto& child : m_children)
        child->Render();
}



void Button::SetDataContext(DataContext* dataContext)
{
    UIElement::SetDataContext(dataContext);
    for (const auto& [key, value] : GetAttributes()) {
        if (key == "Click") {
            m_clickHandler = value;
        } else if (key == "Style") {
            m_style = value;
        } else if (key == "Width") {
            ParseSize(value, m_width);
        } else if (key == "Height") {
            ParseSize(value, m_height);
        } else if (key.find("Trigger_") == 0) {
            std::string triggerName = key.substr(8);
            UIAnimation anim = ParseAnimationString(value);
            if (anim.active)
                m_activeAnimations[triggerName] = anim;
            m_triggers[triggerName] = value;
        } else {
            m_attributes[key] = value;
        }
    }
}

UISize Button::GetElementWidth() const
{
    float availableWidth = m_width.value;
    if (availableWidth <= 0)
        availableWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = availableWidth;
    if (HasAttribute("Width")) {
        try {
            buttonWidth = std::stof(GetAttribute("Width"));
            if (buttonWidth > availableWidth)
                buttonWidth = availableWidth;
        } catch (...) {}
    }
    return UISize(buttonWidth);
}
