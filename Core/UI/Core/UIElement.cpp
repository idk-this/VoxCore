//
// Created by IDKTHIS on 10.10.2025.
//

#include "UIElement.h"

#include <regex>
using namespace UISystem;

inline ImVec4 UIElement::ParseColor(const std::string& str)
{
    std::string color = str;
    std::transform(color.begin(), color.end(), color.begin(), ::tolower);
    color.erase(std::remove_if(color.begin(), color.end(), ::isspace), color.end());

    if (color.starts_with('#')) {
        color = color.substr(1);
        if (color.size() == 3) { // #RGB
            return ImVec4(
                std::stoi(std::string(2, color[0]), nullptr, 16) / 255.0f,
                std::stoi(std::string(2, color[1]), nullptr, 16) / 255.0f,
                std::stoi(std::string(2, color[2]), nullptr, 16) / 255.0f,
                1.0f
            );
        } else if (color.size() == 4) { // #RGBA
            return ImVec4(
                std::stoi(std::string(2, color[0]), nullptr, 16) / 255.0f,
                std::stoi(std::string(2, color[1]), nullptr, 16) / 255.0f,
                std::stoi(std::string(2, color[2]), nullptr, 16) / 255.0f,
                std::stoi(std::string(2, color[3]), nullptr, 16) / 255.0f
            );
        } else if (color.size() == 6) { // #RRGGBB
            return ImVec4(
                std::stoi(color.substr(0, 2), nullptr, 16) / 255.0f,
                std::stoi(color.substr(2, 2), nullptr, 16) / 255.0f,
                std::stoi(color.substr(4, 2), nullptr, 16) / 255.0f,
                1.0f
            );
        } else if (color.size() == 8) { // #RRGGBBAA
            return ImVec4(
                std::stoi(color.substr(0, 2), nullptr, 16) / 255.0f,
                std::stoi(color.substr(2, 2), nullptr, 16) / 255.0f,
                std::stoi(color.substr(4, 2), nullptr, 16) / 255.0f,
                std::stoi(color.substr(6, 2), nullptr, 16) / 255.0f
            );
        }
    }

    // rgba(...) поддержка
    std::regex rgbaRegex(R"(rgba?\s*\(\s*([\d.]+)\s*,\s*([\d.]+)\s*,\s*([\d.]+)(?:\s*,\s*([\d.]+))?\s*\))");
    std::smatch match;
    if (std::regex_match(color, match, rgbaRegex)) {
        float r = std::stof(match[1]) / 255.0f;
        float g = std::stof(match[2]) / 255.0f;
        float b = std::stof(match[3]) / 255.0f;
        float a = match[4].matched ? std::stof(match[4]) : 1.0f;
        if (a > 1.0f) a /= 255.0f;
        return ImVec4(r, g, b, a);
    }

    return ImVec4(1, 1, 1, 1);
}

