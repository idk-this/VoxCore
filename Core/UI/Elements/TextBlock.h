//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "TextElement.h"

namespace UISystem
{
    class TextBlock : public TextElement {
    public:
        TextBlock(const std::string& name);
        void Render() override;
        void ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) override;
        void Update() override;

    private:
        std::string m_style;
        std::string m_bindingPath;
    };
}