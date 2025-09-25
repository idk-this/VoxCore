//
// Created by IDKTHIS on 14.07.2025.
//

#include "VulkanSwapChain.h"

#include "Core/CVar/CVar.h"
#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/RenderPass/VulkanRenderPass.h"

DECLARE_CONVAR("r_vsync_enabled", false, "Enable or disable VSync for the Vulkan swapchain", CVAR_RUNTIME_ONLY | CVAR_ARCHIVE);



VulkanSwapChain::~VulkanSwapChain() {

    for (auto& fb : m_framebuffers) {
        m_context->logicalDevice->GetHandle().destroyFramebuffer(fb);
    }
    m_framebuffers.clear();
    for (auto& view : m_imageViews) {
        m_context->logicalDevice->GetHandle().destroyImageView(view);
    }


    if (m_depthImageView) {
        m_context->logicalDevice->GetHandle().destroyImageView(m_depthImageView);
        m_depthImageView = nullptr;
    }

    if (m_depthImage) {
        m_context->logicalDevice->GetHandle().destroyImage(m_depthImage);
        m_depthImage = nullptr;
    }

    if (m_depthImageMemory) {
        m_context->logicalDevice->GetHandle().freeMemory(m_depthImageMemory);
        m_depthImageMemory = nullptr;
    }
    m_context->logicalDevice->GetHandle().destroySwapchainKHR(m_swapchain);

}

bool VulkanSwapChain::Init(vk::Extent2D extent) {
    auto formats = m_context->physicalDevice->GetHandle().getSurfaceFormatsKHR(m_context->surface);
    auto presentModes = m_context->physicalDevice->GetHandle().getSurfacePresentModesKHR(m_context->surface);
    auto surfaceCaps = m_context->physicalDevice->GetHandle().getSurfaceCapabilitiesKHR(m_context->surface);
    m_surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            m_surfaceFormat = availableFormat;
            break;
            }
    }
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;
    if (GET_CVAR(bool, "r_vsync_enabled")) {
        presentMode = vk::PresentModeKHR::eFifo;
        for (auto pm : presentModes) {
            if (pm == vk::PresentModeKHR::eMailbox) {
                presentMode = vk::PresentModeKHR::eMailbox;
                break;
            }
        }
    }
    m_swapExtent = extent;
    LOG_INFO("Vulkan", "Swapchain surface format: {}, present mode: {}, extent: {}x{}",
             vk::to_string(m_surfaceFormat.format), vk::to_string(presentMode), m_swapExtent.width, m_swapExtent.height);
    uint32_t imageCount = surfaceCaps.minImageCount + 1;
    if (surfaceCaps.maxImageCount > 0 && imageCount > surfaceCaps.maxImageCount) {
        imageCount = surfaceCaps.maxImageCount;
    }
    uint32_t graphicsQueueFamily = m_context->physicalDevice->GetGraphicsQueueFamilyIndex();
    uint32_t presentQueueFamily = m_context->physicalDevice->GetPresentQueueFamilyIndex();

    vk::SharingMode sharingMode;
    std::vector<uint32_t> queueFamilyIndices;

    if (graphicsQueueFamily != presentQueueFamily) {
        sharingMode = vk::SharingMode::eConcurrent;
        queueFamilyIndices = { graphicsQueueFamily, presentQueueFamily };
    } else {
        sharingMode = vk::SharingMode::eExclusive;
    }
    vk::SwapchainCreateInfoKHR swapchainInfo({}, m_context->surface, imageCount, m_surfaceFormat.format,
        m_surfaceFormat.colorSpace, m_swapExtent, 1, vk::ImageUsageFlagBits::eColorAttachment,
        sharingMode, static_cast<uint32_t>(queueFamilyIndices.size()),
        queueFamilyIndices.empty() ? nullptr : queueFamilyIndices.data(),
        surfaceCaps.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode);

    m_swapchain = m_context->logicalDevice->GetHandle().createSwapchainKHR(swapchainInfo);


    auto images = m_context->logicalDevice->GetHandle().getSwapchainImagesKHR(m_swapchain);
    m_imageViews.clear();
    m_imageViews.reserve(images.size());
    for (auto img : images) {
        vk::ImageViewCreateInfo viewInfo({}, img, vk::ImageViewType::e2D, m_surfaceFormat.format,
            {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        m_imageViews.push_back(m_context->logicalDevice->GetHandle().createImageView(viewInfo));
    }
    LOG_INFO("Vulkan",
         "Swapchain created: {} images, format={}, present mode={}, extent={}x{}, sharing={}, queue families: G={}, P={}",
         m_imageViews.size(),
         vk::to_string(m_surfaceFormat.format),
         vk::to_string(presentMode),
         m_swapExtent.width, m_swapExtent.height,
         (sharingMode == vk::SharingMode::eExclusive ? "Exclusive" : "Concurrent"),
         graphicsQueueFamily, presentQueueFamily);

    std::vector<vk::Format> candidates = {
        vk::Format::eD32Sfloat,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16Unorm
    };
    m_depthFormat = m_context->physicalDevice->FindSupportedFormat(candidates, vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    vk::ImageCreateInfo depthImgInfo(
        {},
        vk::ImageType::e2D,
        m_depthFormat,
        vk::Extent3D{m_swapExtent.width, m_swapExtent.height, 1},
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment
    );
    m_depthImage = m_context->logicalDevice->GetHandle().createImage(depthImgInfo);

    vk::MemoryRequirements memReq = m_context->logicalDevice->GetHandle().getImageMemoryRequirements(m_depthImage);

    auto memProps = m_context->physicalDevice->GetHandle().getMemoryProperties();
    uint32_t memTypeIndex = 0;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            memTypeIndex = i;
            break;
            }
    }

    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    m_depthImageMemory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindImageMemory(m_depthImage, m_depthImageMemory, 0);

    vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eDepth;
    if (m_depthFormat == vk::Format::eD24UnormS8Uint || m_depthFormat == vk::Format::eD32SfloatS8Uint) {
        aspectFlags |= vk::ImageAspectFlagBits::eStencil;
    }

    vk::ImageViewCreateInfo depthViewInfo(
        {},
        m_depthImage,
        vk::ImageViewType::e2D,
        m_depthFormat,
        {},
        {aspectFlags, 0, 1, 0, 1}
    );
    m_depthImageView = m_context->logicalDevice->GetHandle().createImageView(depthViewInfo);
    LOG_INFO("Vulkan", "Depth image created: format={}, size={}x{}, memory type={}",
             vk::to_string(m_depthFormat), m_swapExtent.width, m_swapExtent.height,
             vk::to_string(memProps.memoryTypes[memTypeIndex].propertyFlags));
    return true;
}

bool VulkanSwapChain::CreateFramebuffers(VulkanRenderPass* renderPass) {
    for (auto& fb : m_framebuffers) {
        m_context->logicalDevice->GetHandle().destroyFramebuffer(fb);
    }
    m_framebuffers.clear();

    try {
        for (auto& view : m_imageViews) {
            std::array<vk::ImageView, 2> attachments = { view, m_depthImageView };
            vk::FramebufferCreateInfo fbInfo({}, renderPass->GetHandle(), attachments.size(), attachments.data(), m_swapExtent.width, m_swapExtent.height, 1);
            m_framebuffers.push_back(m_context->logicalDevice->GetHandle().createFramebuffer(fbInfo));
        }
    } catch (const std::exception& e) {
        LOG_FATAL("Vulkan", "Failed to create framebuffer: {}", e.what());
        return false;
    }

    return true;
}

void VulkanSwapChain::BeginRender(vk::Semaphore &imageAvalibleSemaphore) {
    m_imageRenderIndex = AcquireNextImage(imageAvalibleSemaphore);
}

void VulkanSwapChain::Present(vk::Semaphore &renderSemaphore) {
    vk::PresentInfoKHR presentInfo(
        1, &renderSemaphore,
        1, &m_swapchain,
        &m_imageRenderIndex
    );
    m_context->logicalDevice->GetGraphicsQueue().presentKHR(presentInfo);
}

uint32_t VulkanSwapChain::AcquireNextImage(vk::Semaphore imageAvailableSemaphore) {
    return m_context->logicalDevice->GetHandle().acquireNextImageKHR(m_swapchain, UINT64_MAX, imageAvailableSemaphore).value;
}

