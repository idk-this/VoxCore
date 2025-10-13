//
// Created by IDKTHIS on 10.10.2025.
//

#pragma once
#include <regex>

#include "Core/UI/Elements/Common/Button.h"
#include "Core/UI/Elements/Layouts/ColumnSystem.h"
#include "Core/UI/Elements/Containers/HUD.h"
#include "Core/UI/Elements/Common/Panel.h"
#include "Core/UI/Elements/Layouts/StackPanel.h"
#include "Core/UI/Elements/Common/TextBlock.h"
#include "UIElement.h"
#include "Core/UI/Elements/Containers/Window.h"

namespace UISystem {

    std::shared_ptr<UIElement> UIElementFactory::CreateElement(const std::string& elementType, const std::string& name) {
        std::shared_ptr<UIElement> element = nullptr;

        if (elementType == "Window") {
            element = std::make_shared<Window>(name);
        }
        else if (elementType == "HUD") {
            element = std::make_shared<HUD>(name);
        }else if (elementType == "TextBlock") {
            element = std::make_shared<TextBlock>(name);
        } else if (elementType == "Button") {
            element = std::make_shared<Button>(name);
        } else if (elementType == "StackPanel") {
            element = std::make_shared<StackPanel>(name);
        } else if (elementType == "Panel") {
            element = std::make_shared<Panel>(name);
        } else if (elementType == "ColumnSystem") {
            element = std::make_shared<ColumnSystem>(name);
        }

        if (element)
            element->SetType(elementType);

        return element;
    }
}
