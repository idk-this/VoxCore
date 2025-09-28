//
// Created by IDKTHIS on 11.09.2025.
//

#pragma once
#include "vulkan/vulkan.hpp"

#include "Core/Export.h"
#include "Platform/Renderer/Vulkan/Core/VulkanContext.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"

struct VulkanContext;
class LogicalDevice;
class PhysicalDevice;

class VOXCORE_API VulkanBuffer {

public:
    VulkanBuffer(VulkanContext* ctx);
    ~VulkanBuffer() {};
    bool Create(vk::DescriptorSetLayout layout, vk::DescriptorPool pool, vk::DeviceSize size, vk::BufferUsageFlags usage, uint32_t binding);
    bool Create(vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    void Destroy();
    uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    template<typename T>
    void UpdateBufferData(const T& data);

    template<typename T>
    void UpdateBufferDataArray(const std::vector<T>& data) {
        void* mapped = m_context->logicalDevice->GetHandle().mapMemory(m_memory, 0, sizeof(T) * data.size());
        std::memcpy(mapped, data.data(), sizeof(T) * data.size());
        m_context->logicalDevice->GetHandle().unmapMemory(m_memory);
    }

    vk::DescriptorSet GetDescriptorSet() const {return m_descriptorSet; }
    void BindBuffer(vk::CommandBuffer* cmd);
    vk::Buffer& GetBuffer() { return m_buffer; }

private:
    VulkanContext* m_context;
    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    vk::DescriptorBufferInfo m_bufferInfo;
    vk::DescriptorSet m_descriptorSet;
};

template <typename T>
void VulkanBuffer::UpdateBufferData(const T& data)
{
    void* mapped = m_context->logicalDevice->GetHandle().mapMemory(m_memory, 0, sizeof(T));
    std::memcpy(mapped, &data, sizeof(T));
    m_context->logicalDevice->GetHandle().unmapMemory(m_memory);
}
