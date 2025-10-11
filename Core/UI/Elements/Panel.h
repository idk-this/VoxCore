//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "UIElement.h"


namespace UISystem
{
    class Panel : public UIElement {
    public:
        Panel(const std::string& name);
        void Render() override;
        void ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) override;
    };
}
