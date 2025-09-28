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
#include "Common/VulkanTexture.h"
#include "Pipeline/GraphicsPipeline.h"
#include "Core/Export.h"
#include "Core/VulkanContext.h"
#include "Core/Common/VoxPak.h"



class UMeshComponent;
class AActor;
class UMesh;
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
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    std::unique_ptr<VulkanBuffer> indexBuffer;
    std::unique_ptr<VulkanBuffer> instanceBuffer;
    std::unique_ptr<VulkanTexture> texture;
    vk::DescriptorSet textureSet;
    vk::DescriptorPool texturePool;
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
    bool UpdateInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors);

    [[nodiscard]] const std::unique_ptr<VulkanContext>& GetContext() const { return m_context; }
private:
    uint32_t m_currentFrame = 0;
    UWorld* m_world = nullptr;
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<VulkanCameraUBO> m_cameraUBO;

    std::unordered_map<UMeshComponent*, MeshRenderData> m_meshDataMap;
    bool InitImGuiForVulkan(IWindow* window);
    IWindow* m_window;
    VoxPak m_shaderPak;

    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;
    vk::DescriptorSetLayout m_textureLayout;
    uint64_t m_renderedVertices = 0;
    std::chrono::high_resolution_clock::time_point m_startTime;
    void Cleanup() override;
    void RenderFrame() override;

};