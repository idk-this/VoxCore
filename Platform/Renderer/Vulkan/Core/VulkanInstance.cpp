//
// Created by IDKTHIS on 07.07.2025.
//

#include "VulkanInstance.h"

#include <vector>

#include "../../../../Core/Log/Logger.h"
#include "SDL3/SDL_vulkan.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
VulkanInstance::VulkanInstance(VulkanContext* context) : m_context(context) {
    m_appInfo = vk::ApplicationInfo{};
    m_appInfo.pApplicationName = "VoxCore Engine base";					// Custom name of the application
    m_appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);		// Custom version of the application
    m_appInfo.pEngineName = "VoxCore";							// Custom engine name
    m_appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);			// Custom engine version
    m_appInfo.apiVersion = VK_API_VERSION_1_3;					// The Vulkan Version

    m_instance = nullptr;
}

VulkanInstance::~VulkanInstance() {
    if (m_instance) {
        m_instance.destroy();
    }
}

bool VulkanInstance::Init() {
    vk::detail::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    if (!vkGetInstanceProcAddr) {
        LOG_FATAL("Vulkan", "Failed to get vkGetInstanceProcAddr");
        return false;
    }
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    // Получение расширений
    uint32_t extCount = 0;
    char const* const* exts = SDL_Vulkan_GetInstanceExtensions(&extCount);
    if (!exts) {
        LOG_FATAL("Vulkan", "Failed to get SDL Vulkan extensions");
        return false;
    }
    std::vector<const char*> extensions(exts, exts + extCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back("VK_KHR_portability_enumeration");
    vk::InstanceCreateInfo instanceInfo({}, &m_appInfo);

    instanceInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_enabledLayers.size());
    instanceInfo.ppEnabledLayerNames = m_enabledLayers.data();

    try {
        m_instance = vk::createInstance(instanceInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
        LOG_INFO("Vulkan", "Vulkan instance created successfully");
        return true;
    } catch (const vk::SystemError& err) {
        LOG_FATAL("Vulkan", "Failed to create instance: {}", err.what());
        return false;
    }


}
