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

GraphicsPipeline::GraphicsPipeline(LogicalDevice* device, PhysicalDevice* physicalDevice,
                                   VulkanRenderPass* renderPass, VulkanSwapChain* swapChain)
    : m_logicalDevice(device),
      m_physicalDevice(physicalDevice),
      m_renderPass(renderPass),
      m_swapChain(swapChain) {

}


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

    vk::Viewport viewport(0, 0, (float)m_swapChain->GetSwapExtent().width, (float)m_swapChain->GetSwapExtent().height, 0, 1);
    vk::Rect2D scissor({0, 0}, m_swapChain->GetSwapExtent());
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

    m_pipelineLayout = m_logicalDevice->GetHandle().createPipelineLayout(layoutInfo);
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
        static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
        &vertexInputInfo, &inputAssembly, nullptr,
        &viewportState, &rasterizer, &multisampling,
        &depthStencil, &colorBlending, nullptr,
        m_pipelineLayout, m_renderPass->GetHandle());
    m_pipeline = m_logicalDevice->GetHandle().createGraphicsPipelines({}, pipelineInfo).value[0];

    return true;
}


void GraphicsPipeline::Cleanup() {
    for (auto & layout : m_descriptorSetLayouts) {
        m_logicalDevice->GetHandle().destroyDescriptorSetLayout(layout);
    }
    m_logicalDevice->GetHandle().destroyPipelineLayout(m_pipelineLayout);

    for (auto & module : m_shader->GetShaderModules()) {
        m_logicalDevice->GetHandle().destroyShaderModule(module.second);
    }
    m_logicalDevice->GetHandle().destroyPipeline(m_pipeline);
}

GraphicsPipeline* GraphicsPipeline::SetPushConstantRange(vk::ShaderStageFlags stages, uint32_t offset, uint32_t size) {
    m_pushConstantRange = vk::PushConstantRange(stages, offset, size);
    return this;
}

GraphicsPipeline * GraphicsPipeline::SetDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout> &layouts) {
    m_descriptorSetLayouts = layouts;
    return this;
}

void GraphicsPipeline::BindDescriptorSet(vk::CommandBuffer* cmd, vk::DescriptorSet descriptorSet)
{
    cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}
