//
// Created by IDKTHIS on 14.07.2025.
//

#pragma once

#include "Platform/Renderer/Vulkan/Core/VulkanInstance.h"

class PhysicalDevice {
public:
    explicit PhysicalDevice(VulkanContext* context)
       : m_context(context) {}
    ~PhysicalDevice();

    // Initialize the physical device with the given Vulkan instance
    bool Init();

    // Get the physical device handle
    vk::PhysicalDevice& GetHandle() { return m_physicalDevice; }
    const vk::PhysicalDeviceFeatures& GetSupportedFeatures() const { return m_supportedFeatures; }

    vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

    // Get the graphics queue family index
    uint32_t& GetGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
    uint32_t& GetPresentQueueFamilyIndex() { return m_presentQueueFamilyIndex; }

private:
    VulkanContext* m_context;
    vk::PhysicalDevice m_physicalDevice; // Handle to the physical device
    vk::PhysicalDeviceFeatures m_supportedFeatures{};
    uint32_t m_graphicsQueueFamilyIndex = -1; // Index of the graphics queue family
    uint32_t m_presentQueueFamilyIndex = -1; // Index of the present queue family
};
