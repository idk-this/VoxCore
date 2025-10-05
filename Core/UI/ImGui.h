//
// Created by IDKTHIS on 25.09.2025.
//

#pragma once
#include <any>
#include <iostream>
#include <stdexcept>

struct ImGuiContext;

enum class ImGuiBackendGraphicsAPI
{
    None,
    Vulkan,
    // OpenGL,
    // DX12,
};

enum class ImGuiBackendWindowAPI
{
    None,
    SDL3,
    // GLFW,
    // Win32,
};

class ImGuiWrapper
{
public:
    ImGuiWrapper(ImGuiBackendGraphicsAPI graphics_api, ImGuiBackendWindowAPI window_api);
    ~ImGuiWrapper();

    void InitGraphics(std::any info);
    void InitWindow(void* windowHandle = nullptr);
    void SetupStyle();
    ImGuiContext* GetContext() { return m_context;}
    void NewFrameGraphics();
    void NewFrameWindow();
    void PollEvents(std::any event);

    void Render(std::any info);
    void Shutdown();

private:
    ImGuiContext* m_context;
    ImGuiBackendGraphicsAPI m_gfxAPI;
    ImGuiBackendWindowAPI   m_wndAPI;
};
