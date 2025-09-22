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
#include "Pipeline/GraphicsPipeline.h"
#include "Core/Export.h"
#include "Core/Common/VoxPak.h"


class AActor;
class UMeshComponent;
class VulkanCommandSystem;
class TrianglePipeline;
class CommandSystem;
class GraphicsPipeline;
class VulkanRenderPass;
class VulkanSwapChain;
class LogicalDevice;
class PhysicalDevice;
class IUISystem;

struct MeshRenderData {
    MeshRenderData(LogicalDevice* logicalDevice, PhysicalDevice* physicalDevice): vertexBuffer(logicalDevice, physicalDevice),
    indexBuffer(logicalDevice, physicalDevice), instanceBuffer(logicalDevice, physicalDevice)
    {

    }
    MeshRenderData(const MeshRenderData&) = delete;
    MeshRenderData& operator=(const MeshRenderData&) = delete;
    MeshRenderData(MeshRenderData&&) = default;
    MeshRenderData& operator=(MeshRenderData&&) = default;
    ~MeshRenderData() = default;
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    VulkanBuffer instanceBuffer;
    uint32_t indexCount = 0;
    uint32_t instanceCount = 0;
};

class VOXCORE_API VulkanRenderer : public IRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;
    bool Init(IWindow* window, UWorld* world) override;
    void BeginFrame() override;
    void ProcessRender();
    void EndFrame() override;
    uint32_t GetCurrentFrame() const { return m_currentFrame; }
    void PrepareMesh(UMeshComponent* mesh, const std::vector<AActor*>& actors);
    void UpdateInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors);

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
    std::unique_ptr<VulkanCameraUBO> m_cameraUBO;

    std::unordered_map<UMeshComponent*, MeshRenderData> m_meshDataMap;

    VoxPak m_shaderPak;

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;

    vk::SurfaceKHR m_surface = nullptr;
    std::chrono::high_resolution_clock::time_point m_startTime;
    void Cleanup() override;
    void RenderFrame() override;

};