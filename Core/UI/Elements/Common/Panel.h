//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "../../Core/UIElement.h"


namespace UISystem
{
    class Panel : public UIElement {
    public:
        Panel(const std::string& name);
        void Render() override;
    };
}
