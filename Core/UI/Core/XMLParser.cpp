//
// Created by IDKTHIS on 10.10.2025.
//

#include "XMLParser.h"
#include <sstream>

#include "StyleManager.h"
#include "Core/Log/Logger.h"

namespace UISystem {
      std::vector<std::string> SplitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);

            if (!token.empty()) {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

    std::shared_ptr<UIElement> XMLUIParser::ParseUI(const std::string& xmlContent, DataContext* dataContext) {
        StyleManager::GetInstance().Clear();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlContent.c_str());

        if (!result) {
            LOG_ERROR("UI System", "XML parsing error: {}", result.description());
            return nullptr;
        }

        return ParseElement(doc.first_child(), dataContext);
    }

    std::shared_ptr<UIElement> XMLUIParser::ParseUIFile(const std::string& filename, DataContext* dataContext) {
        StyleManager::GetInstance().Clear();

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filename.c_str());

        if (!result) {
            LOG_ERROR("UI System", "XML file loading error: {}", result.description());
            return nullptr;
        }

        return ParseElement(doc.first_child(), dataContext);
    }

    std::shared_ptr<UIElement> XMLUIParser::ParseElement(const pugi::xml_node& node, DataContext* dataContext) {
        std::string elementName = node.name();
        std::string nameAttr = node.attribute("Name").as_string();

        if (nameAttr.empty()) {
            nameAttr = elementName;
        }

        auto element = UIElementFactory::CreateElement(elementName, nameAttr);
        if (!element) {
            return nullptr;
        }

        std::string templateFile = node.attribute("Template").as_string();
        if (!templateFile.empty()) {
            StyleManager::GetInstance().LoadStylesFromFile(templateFile);
        }

        auto stylesNode = node.child("Styles");
        if (stylesNode) {
            std::stringstream ss;
            stylesNode.print(ss);
            StyleManager::GetInstance().LoadStylesFromString(ss.str());
        }

        std::unordered_map<std::string, std::string> directAttributes;
        for (auto attr : node.attributes()) {
            std::string attrName = attr.name();
            std::string attrValue = attr.value();

            if (attrName == "Template" || attrName == "Style") continue;

            directAttributes[attrName] = attrValue;
        }

        StyleManager::GetInstance().ApplyDefaultStyles(element);

        std::string styleValue = node.attribute("Style").as_string();
        if (!styleValue.empty()) {
            std::vector<std::string> styleNames = SplitString(styleValue, ',');
            StyleManager::GetInstance().ApplyStyles(styleNames, element);
        }

        for (const auto& [attrName, attrValue] : directAttributes) {
            element->SetAttribute(attrName, attrValue);
        }

        element->SetDataContext(dataContext);

        for (auto child : node.children()) {
            std::string childName = child.name();
            if (childName == "Styles") continue;

            auto childElement = ParseElement(child, dataContext);
            if (childElement) {
                element->AddChild(childElement);
            }
        }

        ApplyBindings(element, dataContext);

        return element;
    }

    void XMLUIParser::ApplyBindings(std::shared_ptr<UIElement> element, DataContext* dataContext) {
        if (!dataContext) return;

        std::function<void(std::shared_ptr<UIElement>)> applyToChildren;
        applyToChildren = [&](std::shared_ptr<UIElement> elem) {
            elem->SetDataContext(dataContext);
            elem->Update();

            for (auto& child : elem->GetChildren()) {
                applyToChildren(child);
            }
        };

        applyToChildren(element);
    }
}
