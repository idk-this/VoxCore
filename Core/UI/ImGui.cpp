//
// Created by IDKTHIS on 25.09.2025.
//

#include "ImGui.h"
#include <backends/imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "Application/Application.h"
#include "libs/imgui/imgui.h"
#include "Platform/Renderer/Vulkan/Common/VulkanBuffer.h"
#include "SDL3/SDL_events.h"

ImGuiWrapper::ImGuiWrapper(ImGuiBackendGraphicsAPI graphics_api, ImGuiBackendWindowAPI window_api)
    : m_gfxAPI(graphics_api),
      m_wndAPI(window_api)
{
    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    SetupStyle();
}

ImGuiWrapper::~ImGuiWrapper()
{

}

void ImGuiWrapper::InitGraphics(std::any info)
{
    switch (m_gfxAPI) {
    case ImGuiBackendGraphicsAPI::Vulkan:
        {
            auto init = std::any_cast<ImGui_ImplVulkan_InitInfo*>(info);
            ImGui_ImplVulkan_Init(init);
        }
        break;
    default: break;
    }
}

void ImGuiWrapper::InitWindow(void* windowHandle)
{
    if (m_gfxAPI == ImGuiBackendGraphicsAPI::Vulkan)
    {
        if (m_wndAPI == ImGuiBackendWindowAPI::SDL3)
        {
            auto sdl_window = static_cast<SDL_Window*>(windowHandle);
            ImGui_ImplSDL3_InitForVulkan(sdl_window);
        }
    }
}

void ImGuiWrapper::SetupStyle()
{
     ImGuiStyle& style = ImGui::GetStyle();
 ImVec4* colors = style.Colors;

 // Rounding
 style.WindowRounding = 0.0f;
 style.ChildRounding = 0.0f;
 style.FrameRounding = 2.0f;
 style.ScrollbarRounding = 2.0f;
 style.GrabRounding = 2.0f;
 style.TabRounding = 2.0f;

 // Border sizes
 style.WindowBorderSize = 1.0f;
 style.FrameBorderSize = 1.0f;
 style.PopupBorderSize = 1.0f;
 style.TabBorderSize = 1.0f;

 // Padding and spacing
 style.WindowPadding = ImVec2(12, 12);
 style.FramePadding = ImVec2(10, 6);
 style.ItemSpacing = ImVec2(10, 6);
 style.ItemInnerSpacing = ImVec2(6, 6);
 style.IndentSpacing = 18.0f;
 style.ScrollbarSize = 14.0f;

 // Color palette
 const ImVec4 ACCENT = ImVec4(0.20f, 0.45f, 0.75f, 1.0f);  // soft blue
 const ImVec4 ACCENT_HOV = ImVec4(0.25f, 0.55f, 0.90f, 1.0f);
 const ImVec4 ACCENT_ACT = ImVec4(0.18f, 0.40f, 0.70f, 1.0f);

 const ImVec4 BG_DARK = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
 const ImVec4 BG_WINDOW = ImVec4(0.09f, 0.09f, 0.09f, 1.0f);
 const ImVec4 BG_FRAME = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
 const ImVec4 BG_HOVER = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
 const ImVec4 BG_ACTIVE = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

 const ImVec4 TEXT_HIGH = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
 const ImVec4 TEXT_MEDIUM = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
 const ImVec4 TEXT_DISABLE = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
 const ImVec4 OUTLINE = ImVec4(1.00f, 1.00f, 1.00f, 0.12f);

 // Text
 colors[ImGuiCol_Text] = TEXT_HIGH;
 colors[ImGuiCol_TextDisabled] = TEXT_DISABLE;

 // Backgrounds
 colors[ImGuiCol_WindowBg] = BG_WINDOW;
 colors[ImGuiCol_ChildBg] = BG_WINDOW;
 colors[ImGuiCol_PopupBg] = BG_DARK;

 // Borders
 colors[ImGuiCol_Border] = OUTLINE;
 colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

 // Frames
 colors[ImGuiCol_FrameBg] = BG_FRAME;
 colors[ImGuiCol_FrameBgHovered] = BG_HOVER;
 colors[ImGuiCol_FrameBgActive] = BG_ACTIVE;

 // Titles
 colors[ImGuiCol_TitleBg] = BG_DARK;
 colors[ImGuiCol_TitleBgActive] = BG_FRAME;
 colors[ImGuiCol_TitleBgCollapsed] = BG_DARK;

 // Menus and headers
 colors[ImGuiCol_MenuBarBg] = BG_DARK;
 colors[ImGuiCol_Header] = BG_FRAME;
 colors[ImGuiCol_HeaderHovered] = BG_HOVER;
 colors[ImGuiCol_HeaderActive] = BG_ACTIVE;

 // Scrollbars
 colors[ImGuiCol_ScrollbarBg] = BG_DARK;
 colors[ImGuiCol_ScrollbarGrab] = BG_FRAME;
 colors[ImGuiCol_ScrollbarGrabHovered] = BG_HOVER;
 colors[ImGuiCol_ScrollbarGrabActive] = BG_ACTIVE;

 // Checkboxes, radio buttons, sliders
 colors[ImGuiCol_CheckMark] = ACCENT;
 colors[ImGuiCol_SliderGrab] = ACCENT;
 colors[ImGuiCol_SliderGrabActive] = ACCENT_ACT;

 // Buttons
 colors[ImGuiCol_Button] = BG_FRAME;
 colors[ImGuiCol_ButtonHovered] = ACCENT_HOV;
 colors[ImGuiCol_ButtonActive] = ACCENT_ACT;

 // Tabs
 colors[ImGuiCol_Tab] = BG_FRAME;
 colors[ImGuiCol_TabHovered] = ACCENT_HOV;
 colors[ImGuiCol_TabActive] = ACCENT;
 colors[ImGuiCol_TabUnfocused] = BG_DARK;
 colors[ImGuiCol_TabUnfocusedActive] = BG_FRAME;

 // Separators
 colors[ImGuiCol_Separator] = OUTLINE;
 colors[ImGuiCol_SeparatorHovered] = ACCENT_HOV;
 colors[ImGuiCol_SeparatorActive] = ACCENT;

 // Resize grips
 colors[ImGuiCol_ResizeGrip] = ACCENT;
 colors[ImGuiCol_ResizeGripHovered] = ACCENT_HOV;
 colors[ImGuiCol_ResizeGripActive] = ACCENT_ACT;

 // Navigation
 colors[ImGuiCol_NavHighlight] = ACCENT;
 colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.70f);
 colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);

 // Plots
 colors[ImGuiCol_PlotLines] = ACCENT;
 colors[ImGuiCol_PlotLinesHovered] = ACCENT_HOV;
 colors[ImGuiCol_PlotHistogram] = ACCENT;
 colors[ImGuiCol_PlotHistogramHovered] = ACCENT_HOV;

 // Tables
 colors[ImGuiCol_TableHeaderBg] = BG_FRAME;
 colors[ImGuiCol_TableBorderStrong] = OUTLINE;
 colors[ImGuiCol_TableBorderLight] = ImVec4(1, 1, 1, 0.05f);
 colors[ImGuiCol_TableRowBg] = ImVec4(1, 1, 1, 0.03f);
 colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.06f);

 // Drag & Drop / Modals
 colors[ImGuiCol_DragDropTarget] = ACCENT;
 colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.60f);

 // Scale for DPI if needed
 /*if (scale != 1.0f)
     style.ScaleAllSizes(scale);*/
}


void ImGuiWrapper::NewFrameGraphics()
{
    switch (m_gfxAPI)
    {
    case ImGuiBackendGraphicsAPI::Vulkan:
        ImGui_ImplVulkan_NewFrame();
        break;
    default: break;
    }
}

void ImGuiWrapper::NewFrameWindow()
{
    switch (m_wndAPI)
    {
    case ImGuiBackendWindowAPI::SDL3:
         ImGui_ImplSDL3_NewFrame();
        break;
    default: break;
    }
}

void ImGuiWrapper::PollEvents(std::any event)
{
    switch (m_wndAPI)
    {
    case ImGuiBackendWindowAPI::SDL3: {
            SDL_Event* sdl_event = std::any_cast<SDL_Event*>(event);
            ImGui_ImplSDL3_ProcessEvent(sdl_event);
            break;
    }
    default:
        break;
    }
}

void ImGuiWrapper::Render(std::any info)
{
    ImGui::Render();
    switch (m_gfxAPI)
    {
    case ImGuiBackendGraphicsAPI::Vulkan:
        {
            auto commandBuffer = std::any_cast<vk::CommandBuffer>(info);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            break;
        }
    default: break;
    }

}

void ImGuiWrapper::Shutdown()
{
    switch (m_gfxAPI)
    {
    case ImGuiBackendGraphicsAPI::Vulkan:
        if (ImGui::GetIO().BackendRendererUserData) {
            ImGui_ImplVulkan_Shutdown();
        }

        break;
    default: break;
    }

    switch (m_wndAPI)
    {
    case ImGuiBackendWindowAPI::SDL3:
        if (ImGui::GetIO().BackendPlatformUserData) {
            ImGui_ImplSDL3_Shutdown();
        }
        break;
    default: break;
    }

    if (ImGui::GetCurrentContext()) {
        ImGui::DestroyContext();
    }
}