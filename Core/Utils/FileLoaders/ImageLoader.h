//
// Created by IDKTHIS on 28.09.2025.
//

#pragma once
#include <memory>
#include <string>
#include <vector>
#include <Core/Export.h>

class UTexture;

namespace Engine
{
    namespace FileLoaders
    {
        struct ImageData {
            int width, height, channels;
            std::vector<unsigned char> pixels;
        };
        class VOXCORE_API ImageLoader {
            public:
                static bool Load(const std::string& path, ImageData& outImage);
                static bool Load(const std::string& path, std::shared_ptr<UTexture>& outTex);
        };

    }
}
