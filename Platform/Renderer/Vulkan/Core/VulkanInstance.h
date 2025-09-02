//
// Created by IDKTHIS on 07.07.2025.
//

#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>


class VulkanInstance {
public:
    VulkanInstance();
    ~VulkanInstance();

    bool Init();
    [[nodiscard]] vk::Instance& GetInstance() { return m_instance; }

private:
    vk::ApplicationInfo m_appInfo;
    vk::Instance m_instance;
    const std::vector<const char*> m_enabledLayers = {
    };
};
