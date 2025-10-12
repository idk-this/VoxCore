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

static float hexToFloat(const std::string& s) {
    return static_cast<float>(std::stoi(s, nullptr, 16)) / 255.0f;
}

ImVec4 TextBlock::ParseColor(const std::string& str) {
    std::string color = str;
    std::transform(color.begin(), color.end(), color.begin(), ::tolower);
    color.erase(std::remove_if(color.begin(), color.end(), ::isspace), color.end());

    if (color.starts_with('#')) {
        color = color.substr(1);
        if (color.size() == 3) {
            return ImVec4(
                hexToFloat(std::string(2, color[0])),
                hexToFloat(std::string(2, color[1])),
                hexToFloat(std::string(2, color[2])),
                1.0f
            );
        } else if (color.size() == 4) {
            return ImVec4(
                hexToFloat(std::string(2, color[0])),
                hexToFloat(std::string(2, color[1])),
                hexToFloat(std::string(2, color[2])),
                hexToFloat(std::string(2, color[3]))
            );
        } else if (color.size() == 6) {
            return ImVec4(
                hexToFloat(color.substr(0, 2)),
                hexToFloat(color.substr(2, 2)),
                hexToFloat(color.substr(4, 2)),
                1.0f
            );
        } else if (color.size() == 8) {
            return ImVec4(
                hexToFloat(color.substr(0, 2)),
                hexToFloat(color.substr(2, 2)),
                hexToFloat(color.substr(4, 2)),
                hexToFloat(color.substr(6, 2))
            );
        }
    }

    std::regex rgbaRegex(R"(rgba?\s*\(\s*([\d.]+)\s*,\s*([\d.]+)\s*,\s*([\d.]+)(?:\s*,\s*([\d.]+))?\s*\))");
    std::smatch match;
    if (std::regex_match(color, match, rgbaRegex)) {
        float r = std::stof(match[1]) / 255.0f;
        float g = std::stof(match[2]) / 255.0f;
        float b = std::stof(match[3]) / 255.0f;
        float a = match[4].matched ? std::stof(match[4]) : 1.0f;
        if (a > 1.0f) a /= 255.0f;
        return ImVec4(r, g, b, a);
    }

    return ImVec4(1, 1, 1, 1);
}

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
