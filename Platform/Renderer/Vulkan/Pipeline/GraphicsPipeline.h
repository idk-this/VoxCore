//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include "IPipeline.h"
#include "glm/vec3.hpp"
#include "../Common/VulkanShader.h"
#include "Platform/Renderer/Vulkan/Swapchain/VulkanSwapChain.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>

class PhysicalDevice;
class LogicalDevice;
class VulkanRenderPass;

class GraphicsPipeline : public IShaderPipeline {
public:

    using IShaderPipeline::IShaderPipeline;

    ~GraphicsPipeline() override {
        GraphicsPipeline::Cleanup();
    }

    bool Init() override;
    void Bind(void* cmdBuffer) const override {};
    void Cleanup() override;
    void SetPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size);
    void SetDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts);
    vk::Pipeline& GetHandle() { return m_pipeline; }
    void BindDescriptorSet(vk::CommandBuffer* cmd, vk::DescriptorSet descriptorSet) override;

private:
};

