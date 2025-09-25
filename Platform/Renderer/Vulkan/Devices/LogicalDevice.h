//
// Created by IDKTHIS on 14.07.2025.
//

#pragma once

#include "Platform/Renderer/Vulkan/Core/VulkanInstance.h"

class PhysicalDevice;

class LogicalDevice {
public:
    explicit LogicalDevice(VulkanContext* context) : m_context(context) {};
    ~LogicalDevice();


    bool Init();

    vk::Device& GetHandle() { return m_logicalDevice; }
    vk::Queue& GetGraphicsQueue() { return m_graphicsQueue; }
    vk::Queue& GetPresentQueue() { return m_presentQueue; }

private:
    VulkanContext* m_context;
    vk::Device m_logicalDevice; // Handle to the logical device
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

};
