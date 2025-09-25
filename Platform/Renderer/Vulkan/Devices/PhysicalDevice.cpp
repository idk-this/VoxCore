//
// Created by IDKTHIS on 14.07.2025.
//

#include "PhysicalDevice.h"

#include "Core/Log/Logger.h"


PhysicalDevice::~PhysicalDevice() {
    m_physicalDevice = nullptr;
}

bool PhysicalDevice::Init() {
    auto gpus =  static_cast<VulkanRenderer*>(Engine::GetCurrentContext().GetRenderer())->GetContext()->instance->GetInstance().enumeratePhysicalDevices();
    vk::PhysicalDeviceMemoryProperties bestMemoryProperties;
    size_t maxVideoMemory = 0;

    for (const auto& gpu : gpus) {
        auto memoryProperties = gpu.getMemoryProperties();
        auto queueProps = gpu.getQueueFamilyProperties();

        int graphicsIndex = -1;
        int presentIndex = -1;

        for (uint32_t i = 0; i < queueProps.size(); ++i) {
            if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
                graphicsIndex = i;
            if (gpu.getSurfaceSupportKHR(i, m_context->surface))
                presentIndex = i;
        }

        if (graphicsIndex == -1 || presentIndex == -1)
            continue;

        size_t totalVideoMemory = 0;
        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
            if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                totalVideoMemory += memoryProperties.memoryHeaps[i].size;
            }
        }

        if (totalVideoMemory > maxVideoMemory) {
            m_physicalDevice = gpu;
            m_graphicsQueueFamilyIndex = graphicsIndex;
            m_presentQueueFamilyIndex = presentIndex;
            maxVideoMemory = totalVideoMemory;
            bestMemoryProperties = memoryProperties;
        }
    }

    if (!m_physicalDevice) {
        LOG_ERROR("Vulkan", "Failed to find a suitable GPU with graphics and present support.");
        return false;
    }
    m_supportedFeatures = m_physicalDevice.getFeatures();
    auto deviceProps = m_physicalDevice.getProperties();
    LOG_INFO("Vulkan", "Selected GPU: {} with {}GB VRAM",
             deviceProps.deviceName.data(), maxVideoMemory / (1024 * 1024 * 1024));
    return true;
}

vk::Format PhysicalDevice::FindSupportedFormat(const std::vector<vk::Format> &candidates, const vk::ImageTiling tiling,
    const vk::FormatFeatureFlags features) const {
    for (const auto& format : candidates) {
        vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);
        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    return vk::Format::eD16Unorm;
}

