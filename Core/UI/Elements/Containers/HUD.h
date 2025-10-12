//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "../../Core/UIElement.h"


namespace UISystem
{
    class HUD : public UIElement {
    public:
        HUD(const std::string& name);
        void Render() override;

    private:
        bool m_visible = true;
    };
}
