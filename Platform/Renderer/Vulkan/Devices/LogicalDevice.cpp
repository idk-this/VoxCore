//
// Created by IDKTHIS on 14.07.2025.
//

#include "LogicalDevice.h"

#include "PhysicalDevice.h"
#include "Core/Log/Logger.h"



LogicalDevice::~LogicalDevice() {
    if (m_logicalDevice) {
        m_logicalDevice.destroy();
    }
}

bool LogicalDevice::Init() {
    uint32_t graphicsQueueFamily = m_context->physicalDevice->GetGraphicsQueueFamilyIndex();
    uint32_t presentQueueFamily =  m_context->physicalDevice->GetPresentQueueFamilyIndex();
    bool sameQueueFamily = (graphicsQueueFamily == presentQueueFamily);

    float priority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    queueInfos.emplace_back(vk::DeviceQueueCreateFlags{}, graphicsQueueFamily, 1, &priority);

    if (!sameQueueFamily) {
        queueInfos.emplace_back(vk::DeviceQueueCreateFlags{}, presentQueueFamily, 1, &priority);
    }
    vk::PhysicalDeviceFeatures enabledFeatures{};
    if (m_context->physicalDevice->GetSupportedFeatures().samplerAnisotropy) {
        enabledFeatures.samplerAnisotropy = VK_TRUE;
    }
    vk::DeviceCreateInfo deviceInfo(
        {},  static_cast<uint32_t>(queueInfos.size()), queueInfos.data(),
        0, nullptr,
        static_cast<uint32_t>(m_deviceExtensions.size()), m_deviceExtensions.data()
    );
    deviceInfo.pEnabledFeatures = &enabledFeatures;
    m_logicalDevice =  m_context->physicalDevice->GetHandle().createDevice(deviceInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_logicalDevice);
    m_graphicsQueue = m_logicalDevice.getQueue( m_context->physicalDevice->GetGraphicsQueueFamilyIndex(), 0);
    m_presentQueue = m_logicalDevice.getQueue( m_context->physicalDevice->GetPresentQueueFamilyIndex(), 0);
    LOG_INFO("Vulkan", "Logical device created with {} extensions.", m_deviceExtensions.size());
    return true;
}
