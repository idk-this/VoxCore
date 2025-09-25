//
// Created by IDKTHIS on 07.07.2025.
//

#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>


struct VulkanContext;

class VulkanInstance {
public:
    explicit VulkanInstance(VulkanContext* context);
    ~VulkanInstance();

    bool Init();
    [[nodiscard]] vk::Instance& GetInstance() { return m_instance; }

private:
    vk::ApplicationInfo m_appInfo;
    VulkanContext* m_context;
    vk::Instance m_instance;
    const std::vector<const char*> m_enabledLayers = {
        //"VK_LAYER_KHRONOS_validation"
    };
};
