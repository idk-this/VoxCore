//
// Created by IDKTHIS on 11.10.2025.
//

#include "TextBlock.h"

#include <imgui.h>
#include <regex>
#include <sstream>
#include <algorithm>

#include "../../Core/UIAnimation.h"

using namespace UISystem;

TextBlock::TextBlock(const std::string& name) : TextElement(name) {}


ImFont* TextBlock::FindFont(const std::string& name, float size) {
    auto& fonts = ImGui::GetIO().Fonts->Fonts;
    for (auto* f : fonts) {
        if (f->Sources.size() > 0) {
            std::string fontName = f->Sources[0]->Name ? f->Sources[0]->Name : "";
            if (!fontName.empty() && name == fontName)
                return f;
        }
    }
    // fallback
    return ImGui::GetFont();
}

void TextBlock::Render() {
    Update();

    ImGui::PushStyleColor(ImGuiCol_Text, m_fontColor);

    if (m_font)
        ImGui::PushFont(m_font);

    if (m_fontSize != 16.0f)
        ImGui::SetWindowFontScale(m_fontSize / 16.0f);

    ImGui::Text("%s", m_text.c_str());

    if (m_fontSize != 16.0f)
        ImGui::SetWindowFontScale(1.0f);

    if (m_font)
        ImGui::PopFont();

    ImGui::PopStyleColor();
}

void TextBlock::Update() {
    if (m_hasBinding && !m_bindingPath.empty() && m_dataContext) {
        std::string value = m_dataContext->GetProperty(m_bindingPath);
        if (!value.empty()) {
            m_text = value;
        }
    }

    if (HasAttribute("FontSize")) {
        m_fontSize = std::stof(GetAttribute("FontSize"));
    }

    if (HasAttribute("FontColor")) {
        m_fontColor = ParseColor(GetAttribute("FontColor"));
    }

    if (HasAttribute("Font")) {
        std::string fontName = GetAttribute("Font");
        if (fontName != m_fontName) {
            m_fontName = fontName;
            m_font = FindFont(m_fontName, m_fontSize);
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
