//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include "TextElement.h"

namespace UISystem
{
     class TextBlock : public TextElement, public DataObserver {
    public:
        TextBlock(const std::string& name);
        ~TextBlock() = default;

        void Render() override;
        void Update() override;
        void SetDataContext(DataContext* dataContext) override;
        void OnDataChanged(const std::string& path, const std::string& value) override;

    private:
        std::string m_bindingPath;
        bool m_hasBinding = false;
    };
}