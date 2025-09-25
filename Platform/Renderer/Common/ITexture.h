//
// Created by IDKTHIS on 09.09.2025.
//

#pragma once

#include <cstdint>

class UTexture;

class ITexture {
public:
    virtual ~ITexture() = default;

    virtual bool UploadFromCPU(const UTexture* src) = 0;

    virtual void Bind(uint32_t slot) = 0;

    [[nodiscard]] virtual int GetWidth() const = 0;
    [[nodiscard]] virtual int GetHeight() const = 0;
    [[nodiscard]] virtual int GetChannels() const = 0;
    virtual void* GetNativeHandle() const = 0;
};