//
// Created by IDKTHIS on 15.07.2025.
//

#include "VulkanCommandSystem.h"

#include "Core/CVar/CVar.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"
#include "Platform/Renderer/Vulkan/RenderPass/VulkanRenderPass.h"
#include "Platform/Renderer/Vulkan/Swapchain/VulkanSwapChain.h"




bool VulkanCommandSystem::Init() {
    vk::CommandPoolCreateInfo poolInfo(
           vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
           m_context->physicalDevice->GetGraphicsQueueFamilyIndex()
       );
    m_commandPool = m_context->logicalDevice->GetHandle().createCommandPool(poolInfo);

    m_commandBuffers = m_context->logicalDevice->GetHandle().allocateCommandBuffers({
        m_commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(GET_CVAR(int, "r_max_frames_in_flight"))
    });

    return true;
}

void VulkanCommandSystem::Cleanup() {

    if (!m_commandBuffers.empty() && m_context->logicalDevice) {
        m_context->logicalDevice->GetHandle().freeCommandBuffers(m_commandPool, m_commandBuffers);
        m_commandBuffers.clear();
    }
    if (m_commandPool && m_context->logicalDevice) {
        m_context->logicalDevice->GetHandle().destroyCommandPool(m_commandPool);
        m_commandPool = nullptr;
    }


}

vk::CommandBuffer VulkanCommandSystem::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer cmdBuffer;
    m_context->logicalDevice->GetHandle().allocateCommandBuffers(&allocInfo, &cmdBuffer);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    cmdBuffer.begin(beginInfo);

    return cmdBuffer;
}

void VulkanCommandSystem::EndSingleTimeCommands(vk::CommandBuffer cmdBuffer)
{
    cmdBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vk::Queue graphicsQueue = m_context->logicalDevice->GetGraphicsQueue();
    graphicsQueue.submit(submitInfo, nullptr);
    graphicsQueue.waitIdle();

    m_context->logicalDevice->GetHandle().freeCommandBuffers(m_commandPool, cmdBuffer);
}
