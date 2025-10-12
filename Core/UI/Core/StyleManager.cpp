//
// Created by IDKTHIS on 12.10.2025.
//

#include "StyleManager.h"

#include "Core/Log/Logger.h"
#include "UIElement.h"

using namespace UISystem;

StyleManager& StyleManager::GetInstance() {
    static StyleManager instance;
    return instance;
}

void StyleManager::LoadStylesFromFile(const std::string& filename) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!result) {
        std::cerr << "Failed to load style file: " << filename << " - " << result.description() << std::endl;
        return;
    }

    auto stylesNode = doc.child("Styles");
    if (stylesNode) {
        ParseStyles(stylesNode);
    }
}

void StyleManager::LoadStylesFromString(const std::string& xmlContent) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlContent.c_str());

    if (!result) {
        std::cerr << "Failed to parse styles from string: " << result.description() << std::endl;
        return;
    }

    auto stylesNode = doc.child("Styles");
    if (stylesNode) {
        ParseStyles(stylesNode);
    }
}

void StyleManager::ParseStyles(const pugi::xml_node& stylesNode) {
    for (auto styleNode : stylesNode.children("Style")) {
        std::string styleName = styleNode.attribute("Name").as_string();
        std::string styleType = styleNode.attribute("Type").as_string();

        if (!styleName.empty()) {
            auto& styleAttributes = m_namedStyles[styleName];

            for (auto setterNode : styleNode.children("Setter")) {
                std::string property = setterNode.attribute("Property").as_string();
                std::string value = setterNode.attribute("Value").as_string();

                if (!property.empty() && !value.empty()) {
                    styleAttributes[property] = value;
                }
            }
        }

        if (!styleType.empty()) {
            auto& styleAttributes = m_typeStyles[styleType];

            for (auto setterNode : styleNode.children("Setter")) {
                std::string property = setterNode.attribute("Property").as_string();
                std::string value = setterNode.attribute("Value").as_string();

                if (!property.empty() && !value.empty()) {
                    styleAttributes[property] = value;
                }
            }
        }
    }
}

void StyleManager::ApplyStyle(const std::string& styleName, std::shared_ptr<UIElement> element) {
    auto it = m_namedStyles.find(styleName);
    if (it == m_namedStyles.end()) return;

    for (const auto& [property, value] : it->second) {
        element->SetAttribute(property, value);
    }
}

void StyleManager::ApplyStyles(const std::vector<std::string>& styleNames, std::shared_ptr<UIElement> element) {
    for (const auto& styleName : styleNames) {
        ApplyStyle(styleName, element);
    }
}

void StyleManager::ApplyDefaultStyles(std::shared_ptr<UIElement> element) {
    std::string elementType = element->GetType();
    auto it = m_typeStyles.find(elementType);
    if (it == m_typeStyles.end()) return;

    for (const auto& [property, value] : it->second) {
        element->SetAttribute(property, value);
    }
}

bool StyleManager::HasStyle(const std::string& styleName) const {
    return m_namedStyles.find(styleName) != m_namedStyles.end();
}

void StyleManager::Clear() {
    m_namedStyles.clear();
    m_typeStyles.clear();
}