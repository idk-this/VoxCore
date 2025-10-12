//
// Created by IDKTHIS on 11.10.2025.
//

#pragma once
#include <string>

struct UIAnimation {
    std::string property;
    float toValue = 0.0f;
    float duration = 0.0f;
    float current = 0.0f;
    bool active = false;
};
