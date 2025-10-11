//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "UIElement.h"


namespace UISystem
{
    class Window : public UIElement {
    public:
        Window(const std::string& name);
        void Render() override;
        void ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) override;

    private:
        std::string m_title;
        int m_flags = 0;
    };
}

