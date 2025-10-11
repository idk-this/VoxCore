//
// Created by IDKTHIS on 11.10.2025.
//

#include "TextBlock.h"

#include <imgui.h>
#include <regex>

using namespace UISystem;

TextBlock::TextBlock(const std::string& name) : TextElement(name) {}

void TextBlock::Render() {
    Update();
    ImGui::Text("%s", m_text.c_str());
}

void TextBlock::ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) {
    auto it = attributes.find("Text");
    if (it != attributes.end()) {
        std::string text = it->second;
        std::regex bindingRegex("\\{Binding Path=([^}]+)\\}");
        std::smatch match;
        if (std::regex_search(text, match, bindingRegex)) {
            m_bindingPath = match[1];
            m_text = "[Binding: " + m_bindingPath + "]";
        } else {
            m_text = text;
        }
    }

    it = attributes.find("Style");
    if (it != attributes.end()) {
        m_style = it->second;
    }
}

void TextBlock::Update() {
    if (!m_bindingPath.empty() && m_dataContext) {
        std::string value = m_dataContext->GetProperty(m_bindingPath);
        if (!value.empty()) {
            m_text = value;
        }
    }
}