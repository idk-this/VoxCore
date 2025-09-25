//
// Created by IDKTHIS on 15.07.2025.
//

#pragma once

#include "Platform/Renderer/Vulkan/Core/VulkanInstance.h"

class GraphicsPipeline;
class VulkanSwapChain;
class VulkanRenderPass;
class LogicalDevice;

class VulkanCommandSystem {
public:
    explicit VulkanCommandSystem(VulkanContext* context) : m_context(context) {};
    ~VulkanCommandSystem()
    {
        Cleanup();
    }
    bool Init();
    void Cleanup();
    vk::CommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(vk::CommandBuffer cmd);
    vk::CommandBuffer& GetCommandBuffer(const int frameIndex = -1) {
        return m_commandBuffers[frameIndex];
    }

private:
    VulkanContext* m_context;
    vk::CommandPool m_commandPool{};
    std::vector<vk::CommandBuffer> m_commandBuffers;
};

