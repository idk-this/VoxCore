//
// Created by IDKTHIS on 15.07.2025.
//

#pragma once

#include "Platform/Renderer/Vulkan/Core/VulkanInstance.h"

class LogicalDevice;
class VulkanSwapChain;

class VulkanRenderPass {
public:
    VulkanRenderPass(VulkanContext* context) : m_context(context) {};
    ~VulkanRenderPass();
    bool Init();
    vk::RenderPass& GetHandle() { return m_renderPass; }

private:
    vk::RenderPass m_renderPass;
    VulkanContext* m_context;
};
