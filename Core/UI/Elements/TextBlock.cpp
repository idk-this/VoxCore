//
// Created by IDKTHIS on 11.10.2025.
//

#include "TextBlock.h"

#include <imgui.h>
#include <regex>

#include "Core/UI/Designer/UIAnimation.h"

using namespace UISystem;

TextBlock::TextBlock(const std::string& name) : TextElement(name) {}

void TextBlock::Render() {
    Update();
    ImGui::Text("%s", m_text.c_str());
}


void TextBlock::Update() {
    if (m_hasBinding && !m_bindingPath.empty() && m_dataContext) {
        std::string value = m_dataContext->GetProperty(m_bindingPath);
        if (!value.empty()) {
            m_text = value;
        }
    }
}
void TextBlock::SetDataContext(DataContext* dataContext) {
    UIElement::SetDataContext(dataContext);
    if (HasAttribute("Text")) {
        std::string text = GetAttribute("Text");

        std::regex bindingRegex("\\{Binding\\s+([^}]+)\\}");
        std::smatch match;
        if (std::regex_search(text, match, bindingRegex)) {
            m_bindingPath = match[1];
            m_hasBinding = true;
            m_text = "[Binding: " + m_bindingPath + "]";
            if (m_dataContext) {
                m_dataContext->AddObserver(m_bindingPath, this);
                Update();
            }
        } else {
            m_text = text;
            m_hasBinding = false;
        }
    }
    if (m_dataContext && !m_bindingPath.empty()) {
        m_dataContext->RemoveObserver(m_bindingPath, this);
    }

    UIElement::SetDataContext(dataContext);

    if (m_dataContext && !m_bindingPath.empty()) {
        m_dataContext->AddObserver(m_bindingPath, this);
        Update();
    }
}

void TextBlock::OnDataChanged(const std::string& path, const std::string& value) {
    if (path == m_bindingPath) {
        m_text = value;
    }
}