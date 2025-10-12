//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once

#include "../../Core/UIElement.h"


namespace UISystem
{
    class TextElement : public UIElement {
    public:
        using UIElement::UIElement;

        void SetText(const std::string& text) { m_text = text; }
        const std::string& GetText() const { return m_text; }

    protected:
        std::string m_text;
    };
}