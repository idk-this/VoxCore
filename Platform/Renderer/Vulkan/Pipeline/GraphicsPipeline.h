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

class GraphicsPipeline : public IPipeline {
public:

    GraphicsPipeline(LogicalDevice* device, PhysicalDevice* physicalDevice,
                      VulkanRenderPass* renderPass, VulkanSwapChain* swapChain);

    ~GraphicsPipeline() override {
        GraphicsPipeline::Cleanup();
    }

    bool Init() override;
    void Bind(void* cmdBuffer) const override {};
    void Cleanup() override;
    GraphicsPipeline* SetShader(VulkanShader* shader) {
        m_shader = shader;
        return this;
    }
    GraphicsPipeline* SetPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size);
    GraphicsPipeline* SetDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts);
    vk::Pipeline& GetHandle() { return m_pipeline; }
    void BindDescriptorSet(vk::CommandBuffer* cmd, vk::DescriptorSet descriptorSet);

private:

    LogicalDevice* m_logicalDevice;
    PhysicalDevice* m_physicalDevice;
    VulkanRenderPass* m_renderPass;
    VulkanSwapChain* m_swapChain;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    VulkanShader* m_shader;
    vk::PushConstantRange m_pushConstantRange{};
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
};

