//
// Created by IDKTHIS on 10.10.2025.
//

#pragma once

#include "UIElement.h"
#include "DataBinding.h"
#include "pugixml.hpp"
#include <memory>

namespace UISystem {
    class VOXCORE_API XMLUIParser {
    public:
        static std::shared_ptr<UIElement> ParseUI(const std::string& xmlContent, DataContext* dataContext = nullptr);
        static std::shared_ptr<UIElement> ParseUIFile(const std::string& filename, DataContext* dataContext = nullptr);

    private:
        static std::shared_ptr<UIElement> ParseElement(const pugi::xml_node& node, DataContext* dataContext);
        static void ApplyBindings(std::shared_ptr<UIElement> element, DataContext* dataContext);
    };
}