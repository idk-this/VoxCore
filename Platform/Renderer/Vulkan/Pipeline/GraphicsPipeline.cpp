//
// Created by IDKTHIS on 16.07.2025.
//

#include "GraphicsPipeline.h"

#include <stdexcept>
#include <cstring>
#include <array>
#include <string>

#include "glm/glm.hpp"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/Devices/PhysicalDevice.h"
#include "Platform/Renderer/Vulkan/RenderPass/VulkanRenderPass.h"
#include "Platform/Renderer/Vulkan/VulkanRenderer.h"
#include "Core/Log/Logger.h"
#include "Platform/Renderer/Common/IShader.h"
#include "Platform/Renderer/Vulkan/Common/VulkanVertexLayout.h"



bool GraphicsPipeline::Init() {

    std::unordered_map<ShaderStage, vk::ShaderModule> shaderStagesMap = m_shader->GetShaderModules();
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    for (const auto& [stage, module] : shaderStagesMap) {
        vk::PipelineShaderStageCreateInfo stageInfo(
           {},
           VulkanShader::ConvertToShaderStage(stage),
           module,
           "main"
       );
        shaderStages.push_back(stageInfo);
    }
    auto layout = std::make_unique<VulkanVertexLayout>();
    auto* binding = static_cast<vk::VertexInputBindingDescription*>(layout->getNativeBindingDescription());
    auto* attrs = static_cast<vk::VertexInputAttributeDescription*>(layout->getNativeAttributeDescriptions());

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
    {},
    static_cast<uint32_t>(layout->bindingDescs.size()), binding,
    static_cast<uint32_t>(layout->attributeDescs.size()), attrs
);
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList);

    vk::Viewport viewport(0, 0, (float)m_context->swapchain->GetSwapExtent().width, (float)m_context->swapchain->GetSwapExtent().height, 0, 1);
    vk::Rect2D scissor({0, 0}, m_context->swapchain->GetSwapExtent());
    vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);
    vk::PipelineRasterizationStateCreateInfo rasterizer({}, false, false,
                     vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise);
    rasterizer.setLineWidth(1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendStateCreateInfo colorBlending({}, false, vk::LogicOp::eCopy, 1, &colorBlendAttachment);
    vk::PipelineDepthStencilStateCreateInfo depthStencil(
        {}, true, true, vk::CompareOp::eLessOrEqual, false, false,
        {}, {}, 0.0f, 1.0f
    );
    vk::PipelineLayoutCreateInfo layoutInfo({},
        static_cast<uint32_t>(m_descriptorSetLayouts.size()), m_descriptorSetLayouts.data(),
        0, nullptr);

    m_pipelineLayout = m_context->logicalDevice->GetHandle().createPipelineLayout(layoutInfo);
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
        static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
        &vertexInputInfo, &inputAssembly, nullptr,
        &viewportState, &rasterizer, &multisampling,
        &depthStencil, &colorBlending, nullptr,
        m_pipelineLayout, m_context->renderPass->GetHandle());
    m_pipeline = m_context->logicalDevice->GetHandle().createGraphicsPipelines({}, pipelineInfo).value[0];

    return true;
}


void GraphicsPipeline::Cleanup() {
    for (auto & layout : m_descriptorSetLayouts) {
        m_context->logicalDevice->GetHandle().destroyDescriptorSetLayout(layout);
    }
    m_context->logicalDevice->GetHandle().destroyPipelineLayout(m_pipelineLayout);

    for (auto & module : m_shader->GetShaderModules()) {
        m_context->logicalDevice->GetHandle().destroyShaderModule(module.second);
    }
    m_context->logicalDevice->GetHandle().destroyPipeline(m_pipeline);
}

void GraphicsPipeline::SetPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) {
    m_pushConstantRange = vk::PushConstantRange(stages, offset, size);
}

void GraphicsPipeline::SetDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout> &layouts) {
    m_descriptorSetLayouts = layouts;
}

void GraphicsPipeline::BindDescriptorSet(vk::CommandBuffer* cmd, vk::DescriptorSet descriptorSet)
{
    cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}
