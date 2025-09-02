//
// Created by IDKTHIS on 02.07.2025.
//

#pragma once
#include <chrono>

#include "../IRenderer.h"
#include <memory>
#include <vector>

#include "Platform/Window/IWindow.h"
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "Core/VulkanInstance.h"
#include "Camera.h"
#include "Pipeline/GraphicsPipeline.h"

struct CameraUBO {
    glm::mat4 view;
    glm::mat4 proj;
};

class VulkanCommandSystem;
class TrianglePipeline;
class CommandSystem;
class GraphicsPipeline;
class VulkanRenderPass;
class VulkanSwapChain;
class LogicalDevice;
class PhysicalDevice;
class IUISystem;

class VulkanRenderer : public IRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;
    bool Init(IWindow* window, UWorld* world) override;
    void BeginFrame() override;
    void ProcessRender();
    void EndFrame() override;

    Camera* GetCamera() { return m_camera.get(); }
    uint32_t GetCurrentFrame() const { return m_currentFrame; }

    [[nodiscard]] const std::unique_ptr<VulkanInstance>& GetInstance() const { return m_instance; }
    [[nodiscard]] const std::unique_ptr<PhysicalDevice>& GetPhysicalDevice() const { return m_physicalDevice; }
    [[nodiscard]] const std::unique_ptr<LogicalDevice>& GetLogicalDevice() const { return m_logicalDevice; }
    [[nodiscard]] const std::unique_ptr<VulkanSwapChain>& GetSwapchain() const { return m_swapchain; }
    [[nodiscard]] const std::unique_ptr<VulkanRenderPass>& GetRenderPass() const { return m_renderPass; }
    [[nodiscard]] const std::unique_ptr<VulkanCommandSystem>& GetCommandSystem() const { return m_commandSystem; }

private:
    uint32_t m_currentFrame = 0;
    UWorld* m_world = nullptr;
    std::unique_ptr<VulkanInstance> m_instance;
    std::unique_ptr<PhysicalDevice> m_physicalDevice;
    std::unique_ptr<LogicalDevice> m_logicalDevice;
    std::unique_ptr<VulkanSwapChain> m_swapchain;
    std::unique_ptr<VulkanRenderPass> m_renderPass;
    std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
    std::unique_ptr<VulkanCommandSystem> m_commandSystem;
    std::unique_ptr<Camera> m_camera;


    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::DescriptorSetLayout m_cameraDescriptorSetLayout;

    vk::SurfaceKHR m_surface = nullptr;
    std::chrono::high_resolution_clock::time_point m_startTime;
    void Cleanup() override;
    void RenderFrame() override;

};