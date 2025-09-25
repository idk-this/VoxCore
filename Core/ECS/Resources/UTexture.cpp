//
// Created by IDKTHIS on 25.09.2025.
//
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "UTexture.h"

UTexture::~UTexture()
{
    Free();
}

bool UTexture::LoadFromFile(const std::string& path)
{
    m_path = path;
    m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
    m_channels = 4;
    return m_data != nullptr;
}


void UTexture::Free() {
    if (m_data) {
        stbi_image_free(m_data);
        m_data = nullptr;
    }
}
