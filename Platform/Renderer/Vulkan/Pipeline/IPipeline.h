//
// Created by IDKTHIS on 13.07.2025.
//
#pragma once
#include "Platform/Renderer/Vulkan/Common/VulkanShader.h"
#include "vulkan/vulkan.hpp"

struct VulkanContext;

enum class PipelineType {
    Graphics,
};

class IPipeline {
public:
    explicit IPipeline(VulkanContext* context) : m_context(context) {}
    virtual ~IPipeline() = default;

    virtual bool Init() = 0;
    virtual void Bind(void* data) const = 0;
    virtual void Cleanup() = 0;
    virtual vk::PipelineLayout& GetLayout() { return m_pipelineLayout; }
    virtual vk::Pipeline& GetPipeline() { return m_pipeline; }
protected:
    VulkanContext* m_context;

    vk::Pipeline m_pipeline{};
    vk::PipelineLayout m_pipelineLayout{};
};

class IShaderPipeline : public IPipeline {
public:
    using IPipeline::IPipeline;

    virtual void SetShader(VulkanShader* shader) {m_shader = shader;};
    virtual void SetPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) = 0;
    virtual void SetDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) = 0;
    virtual void BindDescriptorSet(vk::CommandBuffer* cmd, vk::DescriptorSet descriptorSet) = 0;
protected:
    VulkanShader* m_shader = nullptr;
    vk::PushConstantRange m_pushConstantRange{};
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
};
