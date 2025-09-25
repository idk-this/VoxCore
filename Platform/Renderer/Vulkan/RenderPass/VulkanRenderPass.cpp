//
// Created by IDKTHIS on 15.07.2025.
//


#include "VulkanRenderPass.h"

#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Swapchain/VulkanSwapChain.h"



VulkanRenderPass::~VulkanRenderPass() {
    if (m_renderPass) {
        m_context->logicalDevice->GetHandle().destroyRenderPass(m_renderPass);
    }
}

bool VulkanRenderPass::Init() {
    vk::AttachmentDescription colorAttachment(
        {},
        m_context->swapchain->GetSurfaceFormat().format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentDescription depthAttachment(
        {},
        m_context->swapchain->GetDepthFormat(),
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass(
        {},
        vk::PipelineBindPoint::eGraphics,
        0, nullptr,
        1, &colorRef,
        nullptr,
        &depthRef
    );

    vk::SubpassDependency dependency(
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    );

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    std::array<vk::SubpassDependency, 1> dependencies = { dependency };

    vk::RenderPassCreateInfo renderPassInfo(
        {},
        static_cast<uint32_t>(attachments.size()), attachments.data(),
        1, &subpass,
        static_cast<uint32_t>(dependencies.size()), dependencies.data()
    );

    m_renderPass = m_context->logicalDevice->GetHandle().createRenderPass(renderPassInfo);
    LOG_INFO("Vulkan", "Render pass created with {} attachments.", attachments.size());
    return true;
}
