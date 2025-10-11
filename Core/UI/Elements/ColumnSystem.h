//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "UIElement.h"


namespace UISystem
{
    class ColumnSystem : public UIElement {
    public:
        ColumnSystem(const std::string& name);
        void Render() override;
        void ParseAttributes(const std::unordered_map<std::string, std::string>& attributes) override;
        void AddChild(std::shared_ptr<UIElement> child);
    private:
        int m_columns = 12;
        std::unordered_map<std::string, int> m_childColumnSpans;
        std::vector<int> m_columnSizes;
    };
}
