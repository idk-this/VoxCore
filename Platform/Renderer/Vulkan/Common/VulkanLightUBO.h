//
// Created by IDKTHIS on 14.10.2025.
//

#pragma once
#include "VulkanBuffer.h"
#include "Core/Common/Vertex.h"
#include "Platform/Renderer/Vulkan/Core/VulkanContext.h"


class VulkanLightUBO {
public:
    VulkanLightUBO(VulkanContext* context);
    bool Init();
    void Update(void* cmd, const LightData& data);
    void SetDescriptorSetLayout(vk::DescriptorSetLayout layout) { m_lightDescriptorSetLayout = layout; }
private:
    VulkanContext* m_context;
    VulkanBuffer* m_lightBuffer;
    vk::DescriptorSetLayout m_lightDescriptorSetLayout;
    vk::DescriptorPool m_lightDescriptorPool;
    vk::DescriptorSet m_lightDescriptorSet;
};
