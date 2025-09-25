//
// Created by IDKTHIS on 25.09.2025.
//

#pragma once

#include <string>

#include "Core/ECS/Base/UObject.h"
#include "Core/Export.h"

class VOXCORE_API UTexture : public UObject {
    UCLASS(UTexture);
public:
    UTexture() = default;
    ~UTexture();
    bool LoadFromFile(const std::string& path);
    void Free();
    const unsigned char* GetData() const { return m_data; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }
    const std::string& GetPath() const { return m_path; }

private:
    std::string m_path;
    unsigned char* m_data = nullptr;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};