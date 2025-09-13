//
// Created by IDKTHIS on 02.07.2025.
//

#pragma once

#include <string>
#include <cstdint>
#include <array>

#include "Core/CVar/CVar.h"

CONVAR("w_title", "VoxCore base", "Game window title", CVAR_RUNTIME_ONLY);
CONVAR_MINMAX("w_size_width", 1920, "Game window width", CVAR_ARCHIVE, 320, 15360);
CONVAR_MINMAX("w_size_height", 1080, "Game window height", CVAR_ARCHIVE, 240, 8640);

class WindowInputComponent;

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual bool Create(int width, int height, const std::string& title) {
        m_width = width;
        m_height = height;
        return true;
    };
    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;
    virtual bool ShouldClose() const = 0;

    virtual void* GetNativeHandle() = 0;

    virtual void SetRelativeMouseMode(bool enable) = 0;
    virtual void SetTitle(const std::string& title) = 0;
    [[nodiscard]] uint32_t GetWidth() const { return m_width; }
    [[nodiscard]] uint32_t GetHeight() const { return m_height; }
    WindowInputComponent *GetInputComponent() { return inputComponent; }
protected:
    WindowInputComponent *inputComponent = nullptr;
private:
    uint32_t m_width;
    uint32_t m_height;
};
