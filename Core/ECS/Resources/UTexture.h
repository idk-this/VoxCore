//
// Created by IDKTHIS on 25.09.2025.
//

#pragma once

#include <string>
#include <vector>

#include "Core/ECS/Base/UObject.h"
#include "Core/Export.h"

class VOXCORE_API UTexture : public UObject {
    VClass(UTexture);
public:
    ~UTexture();

    UTexture(const std::string& path,
                int width,
                int height,
                int channels,
                std::vector<unsigned char>&& data)
           : m_path(path),
             m_width(width),
             m_height(height),
             m_channels(channels),
             m_data(std::move(data)) {}
    const unsigned char* GetData() const { return m_data.data(); }
    std::vector<unsigned char>& GetMutableData() { return m_data; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }
    const std::string& GetPath() const { return m_path; }

private:
    std::string m_path;
    std::vector<unsigned char> m_data;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};