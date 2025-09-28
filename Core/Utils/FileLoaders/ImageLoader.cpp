//
// Created by IDKTHIS on 28.09.2025.
//

#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Core/ECS/Resources/UTexture.h"

using namespace Engine::FileLoaders;

bool ImageLoader::Load(const std::string& path, ImageData& outImage)
{
    int w, h, c;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
    if (!data) return false;

    outImage.width = w;
    outImage.height = h;
    outImage.channels = 4;
    outImage.pixels.assign(data, data + w * h * 4);

    stbi_image_free(data);
    return true;
}

bool ImageLoader::Load(const std::string& path, std::shared_ptr<UTexture>& outTex)
{
    ImageData img;
    if (!Load(path, img)) return false;
    outTex = std::make_shared<UTexture>(path, img.width, img.height, img.channels, std::move(img.pixels));
    return true;
}
