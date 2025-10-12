//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "../../Core/UIElement.h"


namespace UISystem
{
    class StackPanel : public UIElement {
    public:
        StackPanel(const std::string& name);
        void Render() override;

    private:
        bool m_horizontal = false;
    };
}
