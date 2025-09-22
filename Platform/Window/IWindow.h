//
// Created by IDKTHIS on 02.07.2025.
//

#pragma once

#include <string>
#include <cstdint>
#include <array>

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
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
