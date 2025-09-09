//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>

#include "Platform/Renderer/Common/IShader.h"


class LogicalDevice;

class VulkanShader : public IShader {
public:
    VulkanShader(LogicalDevice* logicalDevice);
    ~VulkanShader() override;

    void LoadFromFile(const std::string& filepath) override;
    void LoadFromSource(const std::string& source) override;
    const std::unordered_map<ShaderStage, std::vector<uint32_t>>& GetBinaryData() const override;

    const std::unordered_map<ShaderStage, vk::ShaderModule>& GetShaderModules() const;
    vk::ShaderModule& GetShaderModule(const std::string& stageName);
    static shaderc_shader_kind GetShadercKind(ShaderStage stage);
    static vk::ShaderStageFlagBits ConvertToShaderStage(ShaderStage stage);
private:
    LogicalDevice* m_logicalDevice;
    std::unordered_map<ShaderStage, std::vector<uint32_t>> m_spirvCode;
    std::unordered_map<ShaderStage, vk::ShaderModule> m_shaderModules;

    std::unordered_map<std::string, ShaderStage> StringToShaderStage();

    std::vector<uint32_t> CompileGLSL(const std::string& source, shaderc_shader_kind kind);
};
