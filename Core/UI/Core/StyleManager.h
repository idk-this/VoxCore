//
// Created by IDKTHIS on 12.10.2025.
//

#pragma once


#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "pugixml.hpp"

namespace UISystem {
    class UIElement;

    class StyleManager {
    public:
        static StyleManager& GetInstance();

        void LoadStylesFromFile(const std::string& filename);
        void LoadStylesFromString(const std::string& xmlContent);
        void ApplyStyle(const std::string& styleName, std::shared_ptr<UIElement> element);
        void ApplyStyles(const std::vector<std::string>& styleNames, std::shared_ptr<UIElement> element);
        void ApplyDefaultStyles(std::shared_ptr<UIElement> element);
        bool HasStyle(const std::string& styleName) const;
        void Clear();

    private:
        StyleManager() = default;
        void ParseStyles(const pugi::xml_node& stylesNode);

        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_namedStyles;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_typeStyles;
    };
}