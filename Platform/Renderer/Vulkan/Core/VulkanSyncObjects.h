//
// Created by IDKTHIS on 29.09.2025.
//

#pragma once
#include <vulkan/vulkan.hpp>

struct VulkanContext;

class VulkanSyncObjects {
public:
    VulkanSyncObjects(VulkanContext* context);
    ~VulkanSyncObjects();

    bool Create(uint32_t maxFramesInFlight, uint32_t swapchainImageCount);
    void Cleanup();

    void WaitForFrameFence(uint32_t frameIndex);
    void ResetFrameFence(uint32_t frameIndex);

    vk::Semaphore& GetImageAvailableSemaphore(const uint32_t frameIndex) {
        return m_imageAvailableSemaphores[frameIndex];
    }

    vk::Semaphore& GetRenderFinishedSemaphore(const uint32_t imageIndex) {
        return m_renderFinishedSemaphores[imageIndex];
    }

    vk::Fence& GetInFlightFence(const uint32_t frameIndex) {
        return m_inFlightFences[frameIndex];
    }

    vk::SubmitInfo PrepareSubmitInfo(uint32_t frameIndex,
                                    vk::CommandBuffer commandBuffer,
                                    uint32_t imageIndex);

private:
    VulkanContext* m_context;

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;
};
