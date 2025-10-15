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
#include "Common/VulkanCameraUBO.h"
#include "Common/VulkanLightUBO.h"
#include "Common/VulkanTexture.h"
#include "Core/Export.h"
#include "Core/VulkanContext.h"
#include "Core/Common/VoxPak.h"


class VulkanResourceManager;
class VulkanRenderObject;
class UMeshComponent;
class AActor;
class UMesh;
class VulkanCommandSystem;
class TrianglePipeline;
class GraphicsPipeline;
class VulkanRenderPass;
class VulkanSwapChain;
class LogicalDevice;
class PhysicalDevice;
class IUISystem;


class VOXCORE_API VulkanRenderer : public IRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;
    bool Init(IWindow* window, UWorld* world) override;
    void BeginFrame() override;
    void ProcessRender() override;
    void EndFrame() override;
    uint32_t GetCurrentFrame() const { return m_currentFrame; }

    [[nodiscard]] const std::unique_ptr<VulkanContext>& GetContext() const { return m_context; }
private:
    uint32_t m_currentFrame = 0;
    UWorld* m_world = nullptr;
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<VulkanCameraUBO> m_cameraUBO;

    std::unordered_map<UMeshComponent*, std::shared_ptr<VulkanRenderObject>> m_meshDataMap2;
    bool InitImGuiForVulkan(IWindow* window);
    IWindow* m_window;
    std::unique_ptr<VulkanLightUBO> m_lightUBO;
    vk::DescriptorSetLayout m_lightLayout;

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;

    std::unique_ptr<VulkanResourceManager> m_renderObjectManager;
    std::unordered_map<UMeshComponent*, uint32_t> m_meshInstanceCounts;
    std::vector<vk::Fence> m_inFlightFences;
    vk::DescriptorSetLayout m_textureLayout;
    uint64_t m_renderedVertices = 0;
    std::chrono::high_resolution_clock::time_point m_startTime;
    void Cleanup() override;
    void RenderFrame() override;

};