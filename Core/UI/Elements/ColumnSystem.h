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
        void SetDataContext(DataContext* dataContext) override;
        void AddChild(std::shared_ptr<UIElement> child);

    private:
        int m_columns = 1;
    };
}
