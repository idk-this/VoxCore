//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class ShaderStage {
    Vertex,
    Fragment,
    Compute
};

class IShader {
public:
    virtual ~IShader() = default;

    virtual void LoadFromFile(const std::string& filepath) = 0;
    virtual void LoadFromSource(const std::string& source) = 0;

    virtual const std::unordered_map<ShaderStage, std::vector<uint32_t>>& GetBinaryData() const = 0;
};