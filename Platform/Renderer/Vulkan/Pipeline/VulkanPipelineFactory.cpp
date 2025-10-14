//
// Created by IDKTHIS on 13.10.2025.
//

#include "VulkanPipelineFactory.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/RenderPass/VulkanRenderPass.h"

VulkanPipelineFactory::PipelineResult VulkanPipelineFactory::CreatePipeline(VulkanContext* m_ctx,
    const PipelineConfig& cfg)
{
    auto& device = m_ctx->logicalDevice->GetHandle();

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    for (auto& [stage, module] : cfg.shader->GetShaderModules()) {
        shaderStages.push_back({
            {},
            VulkanShader::ConvertToShaderStage(stage),
            module,
            "main"
        });
    }
    std::vector<vk::VertexInputBindingDescription> bindingsDesc;
    std::vector<vk::VertexInputAttributeDescription> attribsDesc;
    uint32_t bindingIndex = 0;
    for (auto& layout : cfg.vertexLayouts) {
        bindingsDesc.emplace_back(bindingIndex, layout.stride, layout.inputRate);
        for (auto& attr : layout.attributes) {
            attribsDesc.emplace_back(attr.location, bindingIndex, attr.format, attr.offset);
        }
        bindingIndex++;
    }
    vk::PipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingsDesc.size());
    vertexInput.pVertexBindingDescriptions = bindingsDesc.data();
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribsDesc.size());
    vertexInput.pVertexAttributeDescriptions = attribsDesc.data();

    uint32_t maxSetIndex = 0;
    for (auto& b : cfg.descriptorBindings) {
        maxSetIndex = std::max(maxSetIndex, b.set);
    }

    std::vector<std::vector<vk::DescriptorSetLayoutBinding>> setBindings(maxSetIndex + 1);
    for (auto& b : cfg.descriptorBindings) {
        setBindings[b.set].emplace_back(b.binding, b.type, b.count, b.stage);
    }

    std::vector<vk::DescriptorSetLayout> setLayouts;
    for (uint32_t i = 0; i < setBindings.size(); ++i) {
        if (!setBindings[i].empty()) {
            vk::DescriptorSetLayoutCreateInfo info{};
            info.bindingCount = static_cast<uint32_t>(setBindings[i].size());
            info.pBindings = setBindings[i].data();
            setLayouts.emplace_back(device.createDescriptorSetLayout(info));
        }
    }


    std::vector<vk::PushConstantRange> vkPushRanges;
    for (auto& pc : cfg.pushConstants) {
        vkPushRanges.emplace_back(pc.stageFlags, pc.offset, pc.size);
    }

    vk::PipelineViewportStateCreateInfo viewportState({}, 1, &cfg.stateConfig.viewport, 1, &cfg.stateConfig.scissor);

    vk::PipelineLayoutCreateInfo layoutInfo({},
        static_cast<uint32_t>(setLayouts.size()), setLayouts.data(),
        vkPushRanges.size(), vkPushRanges.data());

    auto pipelineLayout = m_ctx->logicalDevice->GetHandle().createPipelineLayout(layoutInfo);
    vk::GraphicsPipelineCreateInfo pipelineInfo({},
        static_cast<uint32_t>(shaderStages.size()), shaderStages.data(),
        &vertexInput, &cfg.stateConfig.inputAssembly, nullptr,
        &viewportState, &cfg.stateConfig.rasterization, &cfg.stateConfig.multisample,
        &cfg.stateConfig.depthStencil, &cfg.stateConfig.colorBlend, nullptr,
        pipelineLayout, m_ctx->renderPass->GetHandle());
    auto pipeline = m_ctx->logicalDevice->GetHandle().createGraphicsPipelines({}, pipelineInfo).value[0];
    return {pipeline, pipelineLayout, setLayouts};
}
