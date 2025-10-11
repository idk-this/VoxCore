//
// Created by IDKTHIS on 10.10.2025.
//

#pragma once
#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

#include "../Designer/DataBinding.h"

namespace UISystem {
    enum class SizeType {
        Pixels,
        Percent,
        Auto
    };
    struct UISize {
        float value;
        SizeType type;

        UISize(float v = 0, SizeType t = SizeType::Auto) : value(v), type(t) {}

        float Calculate(float parentSize) const {
            switch(type) {
                case SizeType::Pixels: return value;
                case SizeType::Percent: return parentSize * (value / 100.0f);
                case SizeType::Auto: return parentSize;
            }
            return 0;
        }
    };
    class UIElement {
    public:
        UIElement(const std::string& name) : m_name(name) {}
        virtual ~UIElement() = default;

        virtual void Render() = 0;
        virtual void Update() {}
        virtual void ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) = 0;

        void SetWidth(const UISize& width) { m_width = width; }
        void SetHeight(const UISize& height) { m_height = height; }
        void SetDataContext(DataContext* dataContext) { m_dataContext = dataContext; }
        void SetParent(UIElement* parent) { m_parent = parent; }

        virtual void AddChild(std::shared_ptr<UIElement> child) {
            if (child) {
                child->SetParent(this);
                m_children.push_back(child);
            }
        }
        void SetAttribute(const std::string& name, const std::string& value) {
            m_attributes[name] = value;
        }
        void SetType(const std::string& type) { m_type = type; }
        std::string GetAttribute(const std::string& name) const {
            auto it = m_attributes.find(name);
            return it != m_attributes.end() ? it->second : "";
        }
        bool HasAttribute(const std::string& name) const
        {
            return m_attributes.find(name) != m_attributes.end();
        }
        [[nodiscard]] const std::string& GetType() const { return m_type; }
        const std::unordered_map<std::string, std::string>& GetAttributes() const {
            return m_attributes;
        }

        const std::string& GetName() const { return m_name; }
        UISize GetWidth() const { return m_width; }
        UISize GetHeight() const { return m_height; }
        [[nodiscard]] std::vector<std::shared_ptr<UIElement>> GetChildren() const { return m_children; }

        std::function<void()> OnClick;

    protected:
        std::string m_name;
        std::string m_type;
        UISize m_width;
        UISize m_height;
        std::unordered_map<std::string, std::string> m_attributes;
        DataContext* m_dataContext = nullptr;
        UIElement* m_parent = nullptr;
        std::vector<std::shared_ptr<UIElement>> m_children;
    };

    class UIElementFactory {
    public:
        static std::shared_ptr<UIElement> CreateElement(const std::string& elementType, const std::string& name);
    };

    static void ParseSize(const std::string& sizeStr, UISize& size)
    {
        if (sizeStr.empty()) return;

        if (sizeStr.back() == '%') {
            size.type = SizeType::Percent;
            size.value = std::stof(sizeStr.substr(0, sizeStr.size() - 1));
        } else {
            size.type = SizeType::Pixels;
            size.value = std::stof(sizeStr);
        }
    }
}
