//
// Created by IDKTHIS on 10.10.2025.
//

#include "XMLParser.h"
#include <sstream>

namespace UISystem {
    std::shared_ptr<UIElement> XMLUIParser::ParseUI(const std::string& xmlContent, DataContext* dataContext) {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmlContent.c_str());

        if (!result) {
            return nullptr;
        }

        return ParseElement(doc.first_child(), dataContext);
    }

    std::shared_ptr<UIElement> XMLUIParser::ParseUIFile(const std::string& filename, DataContext* dataContext) {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file(filename.c_str());

        if (!result) {
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

        // Парсинг атрибутов
        std::unordered_map<std::string, std::string> attributes;
        for (auto attr : node.attributes()) {
            std::string attrName = attr.name();
            std::string attrValue = attr.value();

            attributes[attrName] = attrValue;
            element->SetAttribute(attrName, attrValue); // Сохраняем атрибуты в элементе
        }

        element->ParseAttributes(attributes);
        element->SetDataContext(dataContext);

        // Рекурсивный парсинг дочерних элементов
        for (auto child : node.children()) {
            auto childElement = ParseElement(child, dataContext);
            if (childElement) {
                element->AddChild(childElement);
            }
        }

        // Применение биндингов
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