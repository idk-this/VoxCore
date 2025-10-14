//
// Created by IDKTHIS on 13.10.2025.
//

#pragma once
#include "Platform/Renderer/Vulkan/Common/VulkanShader.h"
#include "Platform/Renderer/Vulkan/Common/VulkanVertexLayout.h"
#include "Platform/Renderer/Vulkan/Core/VulkanContext.h"



struct PushConstantRange {
    vk::ShaderStageFlags stageFlags;
    uint32_t offset;
    uint32_t size;
};

struct PipelineStateConfig {
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    vk::PipelineRasterizationStateCreateInfo rasterization{};
    vk::PipelineColorBlendStateCreateInfo colorBlend{};
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    vk::PipelineMultisampleStateCreateInfo multisample{};
    vk::Viewport viewport{};
    vk::Rect2D scissor{};
};
struct DescriptorBinding {
    uint32_t set;
    uint32_t binding;
    vk::DescriptorType type;
    vk::ShaderStageFlags stage;
    uint32_t count = 1;
};

struct VertexAttribute {
    uint32_t location;
    vk::Format format;
    uint32_t offset;
};

struct VertexInputLayout {
    uint32_t stride = 0;
    std::vector<VertexAttribute> attributes;
    vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex;
};
struct PipelineConfig {
    VulkanShader* shader = nullptr;
    std::vector<DescriptorBinding> descriptorBindings;
    std::vector<VertexInputLayout> vertexLayouts;
    std::vector<PushConstantRange> pushConstants;

    PipelineStateConfig stateConfig;
    vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
    bool blendEnable = false;
    bool depthTest = true;
};

class VulkanPipelineFactory {
public:
    struct PipelineResult {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    };

    static PipelineResult CreatePipeline(VulkanContext* m_ctx, const PipelineConfig& cfg);
};
