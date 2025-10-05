//
// Created by IDKTHIS on 29.09.2025.
//

#include "VulkanSyncObjects.h"
#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"

VulkanSyncObjects::VulkanSyncObjects(VulkanContext* context)
    : m_context(context)
{
}

VulkanSyncObjects::~VulkanSyncObjects() {
    Cleanup();
}

bool VulkanSyncObjects::Create(uint32_t maxFramesInFlight, uint32_t swapchainImageCount) {
    try {
        m_imageAvailableSemaphores.resize(maxFramesInFlight);
        m_renderFinishedSemaphores.resize(swapchainImageCount);
        m_inFlightFences.resize(maxFramesInFlight);

        auto& device = m_context->logicalDevice->GetHandle();

        // Create semaphores and fences
        for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
            m_imageAvailableSemaphores[i] = device.createSemaphore({});
            m_inFlightFences[i] = device.createFence({vk::FenceCreateFlagBits::eSignaled});
        }

        for (uint32_t i = 0; i < swapchainImageCount; ++i) {
            m_renderFinishedSemaphores[i] = device.createSemaphore({});
        }

        LOG_INFO("Vulkan", "Created sync objects: {} frames in flight, {} swapchain images",
                 maxFramesInFlight, swapchainImageCount);
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Vulkan", "Failed to create sync objects: {}", e.what());
        Cleanup();
        return false;
    }
}

void VulkanSyncObjects::Cleanup() {
    auto& device = m_context->logicalDevice->GetHandle();

    for (auto& semaphore : m_imageAvailableSemaphores) {
        if (semaphore) {
            device.destroySemaphore(semaphore);
        }
    }
    m_imageAvailableSemaphores.clear();

    for (auto& semaphore : m_renderFinishedSemaphores) {
        if (semaphore) {
            device.destroySemaphore(semaphore);
        }
    }
    m_renderFinishedSemaphores.clear();

    for (auto& fence : m_inFlightFences) {
        if (fence) {
            device.destroyFence(fence);
        }
    }
    m_inFlightFences.clear();
}

void VulkanSyncObjects::WaitForFrameFence(uint32_t frameIndex) {
    if (frameIndex < m_inFlightFences.size()) {
        m_context->logicalDevice->GetHandle().waitForFences(
            m_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
    }
}

void VulkanSyncObjects::ResetFrameFence(uint32_t frameIndex) {
    if (frameIndex < m_inFlightFences.size()) {
        m_context->logicalDevice->GetHandle().resetFences(m_inFlightFences[frameIndex]);
    }
}

vk::SubmitInfo VulkanSyncObjects::PrepareSubmitInfo(uint32_t frameIndex,
                                                    vk::CommandBuffer commandBuffer,
                                                    uint32_t imageIndex) {
    vk::Semaphore waitSemaphore = m_imageAvailableSemaphores[frameIndex];
    vk::Semaphore signalSemaphore = m_renderFinishedSemaphores[imageIndex];

    return vk::SubmitInfo{}
    .setWaitSemaphores(waitSemaphore)
    .setCommandBuffers(commandBuffer)
    .setSignalSemaphores(signalSemaphore);
}
