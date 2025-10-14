//
// Created by IDKTHIS on 14.10.2025.
//

#include "VulkanLightUBO.h"

#include "Core/Common/Vertex.h"

VulkanLightUBO::VulkanLightUBO(VulkanContext* context)
{
    m_lightBuffer = new VulkanBuffer(context);
    m_context = context;
}

bool VulkanLightUBO::Init()
{
    vk::DescriptorPoolSize poolSize{vk::DescriptorType::eUniformBuffer, 1};
    vk::DescriptorPoolCreateInfo poolInfo{{}, 1, 1, &poolSize};
    m_lightDescriptorPool = m_context->logicalDevice->GetHandle().createDescriptorPool(poolInfo);

    m_lightBuffer->Create(
        m_lightDescriptorSetLayout, m_lightDescriptorPool, sizeof(LightData),
        vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_lightDescriptorSet = m_lightBuffer->GetDescriptorSet();
    return true;
}

void VulkanLightUBO::Update(void* cmd, const LightData& data)
{
    m_lightBuffer->UpdateBufferData(data);
    vk::CommandBuffer commandBuffer = *reinterpret_cast<vk::CommandBuffer*>(cmd);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_context->pipelines[PipelineType::Graphics].layout,
        2,
        m_lightDescriptorSet,
        nullptr
    );
}
