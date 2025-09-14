//
// Created by IDKTHIS on 05.09.2025.
//

#include "VulkanCameraUBO.h"

#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/Pipeline/GraphicsPipeline.h"

VulkanCameraUBO::VulkanCameraUBO(LogicalDevice* logicalDevice, PhysicalDevice* physicalDevice)
{
    m_cameraBuffer = new VulkanBuffer(logicalDevice, physicalDevice);
    m_logicalDevice = logicalDevice;

}

bool VulkanCameraUBO::PreInit(GraphicsPipeline* pipeline)
{
    vk::DescriptorSetLayoutBinding cameraLayoutBinding{};
    cameraLayoutBinding.binding = 0;
    cameraLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    cameraLayoutBinding.pImmutableSamplers = nullptr;
    vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, 1, &cameraLayoutBinding};
    m_cameraDescriptorSetLayout = m_logicalDevice->GetHandle().createDescriptorSetLayout(layoutInfo);
    m_graphicsPipeline = pipeline;
    return true;
}

bool VulkanCameraUBO::Init()
{
    vk::DescriptorPoolSize poolSize{vk::DescriptorType::eUniformBuffer, 1};
    vk::DescriptorPoolCreateInfo poolInfo{{}, 1, 1, &poolSize};
    m_cameraDescriptorPool = m_logicalDevice->GetHandle().createDescriptorPool(poolInfo);

    m_cameraBuffer->Create(
        m_cameraDescriptorSetLayout, m_cameraDescriptorPool, sizeof(CameraData),
        vk::BufferUsageFlagBits::eUniformBuffer, 0);
    m_cameraDescriptorSet = m_cameraBuffer->GetDescriptorSet();

    return true;
}

void VulkanCameraUBO::Update(void* cmd, const CameraData& data)
{
    m_cameraBuffer->UpdateBufferData(data);
    m_graphicsPipeline->BindDescriptorSet((vk::CommandBuffer*)cmd, m_cameraDescriptorSet);
}
