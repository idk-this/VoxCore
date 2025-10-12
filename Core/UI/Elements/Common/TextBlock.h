//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "TextElement.h"
#include <string>

#include "imgui.h"

namespace UISystem
{
    class TextBlock : public TextElement, public DataObserver {
    public:
        TextBlock(const std::string& name);
        ~TextBlock() = default;

        void Render() override;
        void Update() override;
        void SetDataContext(DataContext* dataContext) override;
        void OnDataChanged(const std::string& path, const std::string& value) override;

    private:
        std::string m_bindingPath;
        bool m_hasBinding = false;

        float m_fontSize = 16.0f;
        ImVec4 m_fontColor = ImVec4(1, 1, 1, 1);
        std::string m_fontName;
        ImFont* m_font = nullptr;

        ImVec4 ParseColor(const std::string& str);
        ImFont* FindFont(const std::string& name, float size);
    };
}
