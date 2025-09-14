//
// Created by IDKTHIS on 11.09.2025.
//

#include "VulkanBuffer.h"

#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/Devices/PhysicalDevice.h"

VulkanBuffer::VulkanBuffer(LogicalDevice* logicalDevice, PhysicalDevice* physicalDevice)
    : m_logicalDevice(logicalDevice), m_physicalDevice(physicalDevice)
{
}

bool VulkanBuffer::Create(vk::DescriptorSetLayout layout, vk::DescriptorPool pool,
    vk::DeviceSize size, vk::BufferUsageFlags usage, uint32_t binding)
{
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    m_buffer = m_logicalDevice->GetHandle().createBuffer(bufferInfo);

    vk::MemoryRequirements memReq = m_logicalDevice->GetHandle().getBufferMemoryRequirements(m_buffer);
    uint32_t memTypeIndex = FindMemoryType(memReq.memoryTypeBits,
                                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    m_memory = m_logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_logicalDevice->GetHandle().bindBufferMemory(m_buffer, m_memory, 0);

    vk::DescriptorSetAllocateInfo allocInfoSet(pool, 1, &layout);
    m_descriptorSet = m_logicalDevice->GetHandle().allocateDescriptorSets(allocInfoSet).front();

    m_bufferInfo = vk::DescriptorBufferInfo(m_buffer, 0, size);
    vk::WriteDescriptorSet write(m_descriptorSet, binding, 0, 1,
                                 usage == vk::BufferUsageFlagBits::eUniformBuffer ?
                                 vk::DescriptorType::eUniformBuffer : vk::DescriptorType::eStorageBuffer,
                                 nullptr, &m_bufferInfo);
    m_logicalDevice->GetHandle().updateDescriptorSets(1, &write, 0, nullptr);
    return true;
}

uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    auto memProperties = m_physicalDevice->GetHandle().getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    LOG_ERROR("Vulkan", "Failed to find suitable memory type!");
    return -1;
}
