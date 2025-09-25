//
// Created by IDKTHIS on 09.09.2025.
//

#pragma once
#include <vulkan/vulkan.hpp>

#include "Platform/Renderer/Common/ITexture.h"


struct VulkanContext;

class VulkanTexture : public ITexture {
public:
    explicit VulkanTexture(VulkanContext* context) : m_context(context){};
    ~VulkanTexture();

    bool UploadFromCPU(const UTexture* src) override;
    void Bind(uint32_t slot) override {}

    [[nodiscard]] int GetWidth() const override { return m_width; }
    [[nodiscard]] int GetHeight() const override { return m_height; }
    [[nodiscard]] int GetChannels() const override { return m_channels; }

    void* GetNativeHandle() const override { return (void*)m_imageView; }

    vk::Image GetImage() const { return m_image; }
    vk::ImageView GetImageView() const { return m_imageView; }
    vk::Sampler GetSampler() const { return m_sampler; }

private:
    bool CreateImage(int width, int height, vk::Format format);
    bool AllocateMemory(vk::MemoryPropertyFlags properties);
    void TransitionLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void CopyBufferToImage(vk::Buffer buffer, int width, int height);

private:

    VulkanContext* m_context;
    vk::Image m_image{};
    vk::DeviceMemory m_memory{};
    vk::ImageView m_imageView{};
    vk::Sampler m_sampler{};

    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};
