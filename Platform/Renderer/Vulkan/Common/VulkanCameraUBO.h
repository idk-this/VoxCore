//
// Created by IDKTHIS on 05.09.2025.
//

#pragma once
#include "Platform/Renderer/Common/ICameraUBO.h"
#include <vulkan/vulkan.hpp>

#include "VulkanBuffer.h"
#include "Core/Export.h"

class GraphicsPipeline;
class LogicalDevice;

class VOXCORE_API VulkanCameraUBO : public ICameraUBO {
public:
    VulkanCameraUBO(LogicalDevice* logicalDevice, PhysicalDevice* physicalDevice);
    bool PreInit(GraphicsPipeline* pipeline);
    bool Init();
    void Update(void* cmd, const CameraData& data) override;
    vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_cameraDescriptorSetLayout;}
    vk::DescriptorSet& GetDescriptorSet() { return m_cameraDescriptorSet; }
    VulkanBuffer* GetBuffer() const { return m_cameraBuffer; }

private:
    GraphicsPipeline* m_graphicsPipeline;
    LogicalDevice* m_logicalDevice;
    VulkanBuffer* m_cameraBuffer;
    vk::DescriptorSetLayout m_cameraDescriptorSetLayout;
    vk::DescriptorPool m_cameraDescriptorPool;
    vk::DescriptorSet m_cameraDescriptorSet;
};
