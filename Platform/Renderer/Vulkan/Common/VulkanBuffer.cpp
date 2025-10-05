//
// Created by IDKTHIS on 11.09.2025.
//

#include "VulkanBuffer.h"

#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/Devices/PhysicalDevice.h"

VulkanBuffer::VulkanBuffer(VulkanContext* ctx)
    : m_context(ctx)
{
}

bool VulkanBuffer::Create(vk::DescriptorSetLayout layout, vk::DescriptorPool pool,
    vk::DeviceSize size, vk::BufferUsageFlags usage, uint32_t binding)
{
    vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);
    m_buffer = m_context->logicalDevice->GetHandle().createBuffer(bufferInfo);

    vk::MemoryRequirements memReq = m_context->logicalDevice->GetHandle().getBufferMemoryRequirements(m_buffer);
    uint32_t memTypeIndex = FindMemoryType(memReq.memoryTypeBits,
                                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    m_memory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindBufferMemory(m_buffer, m_memory, 0);

    vk::DescriptorSetAllocateInfo allocInfoSet(pool, 1, &layout);
    m_descriptorSet = m_context->logicalDevice->GetHandle().allocateDescriptorSets(allocInfoSet).front();

    m_bufferInfo = vk::DescriptorBufferInfo(m_buffer, 0, size);
    vk::WriteDescriptorSet write(m_descriptorSet, binding, 0, 1,
                                 usage == vk::BufferUsageFlagBits::eUniformBuffer ?
                                 vk::DescriptorType::eUniformBuffer : vk::DescriptorType::eStorageBuffer,
                                 nullptr, &m_bufferInfo);
    m_context->logicalDevice->GetHandle().updateDescriptorSets(1, &write, 0, nullptr);
    return true;
}

bool VulkanBuffer::Create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    m_buffer = m_context->logicalDevice->GetHandle().createBuffer(bufferInfo);

    vk::MemoryRequirements memRequirements = m_context->logicalDevice->GetHandle().getBufferMemoryRequirements(m_buffer);
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    m_memory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindBufferMemory(m_buffer, m_memory, 0);
    m_size = size;

    return true;
}

void VulkanBuffer::Destroy()
{
    if (m_buffer) {
        try {
            m_context->logicalDevice->GetHandle().destroyBuffer(m_buffer);
        } catch (...) {}
        m_buffer = VK_NULL_HANDLE;
    }

    if (m_memory) {
        try {
            m_context->logicalDevice->GetHandle().freeMemory(m_memory);
        } catch (...) {}
        m_memory = VK_NULL_HANDLE;
    }


    m_descriptorSet = VK_NULL_HANDLE;
    m_bufferInfo = vk::DescriptorBufferInfo();
}

uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    auto memProperties = m_context->physicalDevice->GetHandle().getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    LOG_ERROR("Vulkan", "Failed to find suitable memory type!");
    return -1;
}

void VulkanBuffer::BindBuffer(vk::CommandBuffer* cmd)
{
    VkDeviceSize offset = 0;
    cmd->bindVertexBuffers(0, 1, &m_buffer, &offset);
}
