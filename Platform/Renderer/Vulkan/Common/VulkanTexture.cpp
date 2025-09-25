//
// Created by IDKTHIS on 09.09.2025.
//

#include "VulkanTexture.h"

#include "Core/ECS/Resources/UTexture.h"
#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/VulkanRenderer.h"
#include "Platform/Renderer/Vulkan/Commands/VulkanCommandSystem.h"

VulkanTexture::~VulkanTexture()
{
    if (!m_context || !m_context->logicalDevice) return; // защита

    auto device = m_context->logicalDevice->GetHandle();

    if (m_sampler) {
        device.destroySampler(m_sampler);
        m_sampler = nullptr;
    }

    if (m_imageView) {
        device.destroyImageView(m_imageView);
        m_imageView = nullptr;
    }

    if (m_image) {
        device.destroyImage(m_image);
        m_image = nullptr;
    }

    if (m_memory) {
        device.freeMemory(m_memory);
        m_memory = nullptr;
    }
}


bool VulkanTexture::UploadFromCPU(const UTexture* src)
{
    if (!src || !src->GetData()) {
        LOG_ERROR("VulkanTexture", "Invalid source texture!");
        return false;
    }

    m_width = src->GetWidth();
    m_height = src->GetHeight();
    m_channels = src->GetChannels();

    vk::DeviceSize imageSize = m_width * m_height * m_channels;

    // 1. Создаем staging buffer
    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingMemory;

    vk::BufferCreateInfo bufferInfo({}, imageSize,
                                    vk::BufferUsageFlagBits::eTransferSrc,
                                    vk::SharingMode::eExclusive);

    stagingBuffer = m_context->logicalDevice->GetHandle().createBuffer(bufferInfo);
    vk::MemoryRequirements memReq = m_context->logicalDevice->GetHandle().getBufferMemoryRequirements(stagingBuffer);

    uint32_t memTypeIndex = 0;
    auto memProps = m_context->physicalDevice->GetHandle().getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)) {
            memTypeIndex = i;
            break;
        }
    }

    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    stagingMemory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindBufferMemory(stagingBuffer, stagingMemory, 0);

    void* data = m_context->logicalDevice->GetHandle().mapMemory(stagingMemory, 0, imageSize);
    memcpy(data, src->GetData(), static_cast<size_t>(imageSize));
    m_context->logicalDevice->GetHandle().unmapMemory(stagingMemory);

    // 2. Создаем vk::Image
    vk::Format format = (m_channels == 4) ? vk::Format::eR8G8B8A8Unorm : vk::Format::eR8G8B8Unorm;
    if (!CreateImage(m_width, m_height, format)) {
        return false;
    }

    // 3. Layout transitions + copy
    TransitionLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(stagingBuffer, m_width, m_height);
    TransitionLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    // 4. ImageView
    vk::ImageViewCreateInfo viewInfo({}, m_image, vk::ImageViewType::e2D, format,
                                     {},
                                     { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    m_imageView = m_context->logicalDevice->GetHandle().createImageView(viewInfo);

    // 5. Sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

    m_sampler = m_context->logicalDevice->GetHandle().createSampler(samplerInfo);

    // 6. Чистим staging
    m_context->logicalDevice->GetHandle().destroyBuffer(stagingBuffer);
    m_context->logicalDevice->GetHandle().freeMemory(stagingMemory);

    return true;
}

bool VulkanTexture::CreateImage(int width, int height, vk::Format format)
{
    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    m_image = m_context->logicalDevice->GetHandle().createImage(imageInfo);

    vk::MemoryRequirements memReq = m_context->logicalDevice->GetHandle().getImageMemoryRequirements(m_image);

    uint32_t memTypeIndex = 0;
    auto memProps = m_context->physicalDevice->GetHandle().getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
            memTypeIndex = i;
            break;
            }
    }

    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    m_memory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindImageMemory(m_image, m_memory, 0);

    return true;
}

bool VulkanTexture::AllocateMemory(vk::MemoryPropertyFlags properties)
{
    vk::MemoryRequirements memReq = m_context->logicalDevice->GetHandle().getImageMemoryRequirements(m_image);

    uint32_t memTypeIndex = UINT32_MAX;
    auto memProps = m_context->physicalDevice->GetHandle().getMemoryProperties();
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties)) {
            memTypeIndex = i;
            break;
            }
    }

    if (memTypeIndex == UINT32_MAX) {
        LOG_ERROR("VulkanTexture", "Failed to find suitable memory type!");
        return false;
    }

    vk::MemoryAllocateInfo allocInfo(memReq.size, memTypeIndex);
    m_memory = m_context->logicalDevice->GetHandle().allocateMemory(allocInfo);
    m_context->logicalDevice->GetHandle().bindImageMemory(m_image, m_memory, 0);

    return true;
}

void VulkanTexture::TransitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer cmd = m_context->commandSystem->BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else {
        LOG_ERROR("VulkanTexture", "Unsupported layout transition!");
        m_context->commandSystem->EndSingleTimeCommands(cmd);
        return;
    }

    cmd.pipelineBarrier(srcStage, dstStage, {}, nullptr, nullptr, barrier);
    m_context->commandSystem->EndSingleTimeCommands(cmd);
}

void VulkanTexture::CopyBufferToImage(vk::Buffer buffer, int width, int height)
{
    vk::CommandBuffer cmd = m_context->commandSystem->BeginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{static_cast<uint32_t>(width),
                                      static_cast<uint32_t>(height),
                                      1};

    cmd.copyBufferToImage(buffer, m_image,
                          vk::ImageLayout::eTransferDstOptimal,
                          { region });

    m_context->commandSystem->EndSingleTimeCommands(cmd);
}
