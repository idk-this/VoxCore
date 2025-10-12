//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "../../Core/UIElement.h"


namespace UISystem
{
    class Window : public UIElement {
    public:
        Window(const std::string& name);
        void Render() override;

    private:
        std::string m_title = "Window";
        int m_flags = 0;
        bool m_canClose = true;
        bool m_visible = true;
    };
}

