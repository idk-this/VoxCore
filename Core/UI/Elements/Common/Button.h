//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include <map>

#include "../../Core/UIElement.h"
#include "Core/UI/Core/UIAnimation.h"


namespace UISystem
{
    class Button : public UIElement {
    public:
        Button(const std::string& name);
        void Render() override;
        void SetDataContext(DataContext* dataContext) override;
        UISize GetElementWidth() const override;
    private:
        std::string m_clickHandler;
        std::string m_style;
        std::map<std::string, std::string> m_triggers;
        std::unordered_map<std::string, UIAnimation> m_activeAnimations;
    };
}
