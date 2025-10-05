//
// Created by IDKTHIS on 29.09.2025.
//

#pragma once


#include "Platform/Renderer/Common/IRenderResourceManager.h"
#include "VulkanRenderObject.h"
#include "Platform/Renderer/Vulkan/Common/VulkanCameraUBO.h"


class UWorld;
class VulkanContext;

class VulkanResourceManager : public IRenderResourceManager {
public:
    VulkanResourceManager(VulkanContext* context);
    ~VulkanResourceManager() override;

    bool Initialize() override;
    void Cleanup() override;

    void UpdateWorldState(UWorld* world);

    const std::vector<std::shared_ptr<VulkanRenderObject>>& GetRenderObjects() const { return m_renderObjects; }

    void RenderObjects(vk::CommandBuffer& cmd, VulkanCameraUBO* cameraUBO, uint32_t currentFrame);


private:
    void OnActorSpawn(AActor* actor);
    void OnActorDestroy(AActor* actor);
    void ProcessMeshComponent(AActor* actor, UMeshComponent* mesh);
    void RemoveMeshComponent(UMeshComponent* mesh);
    void UpdateMeshComponent(UMeshComponent* mesh);

    void PrepareInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors);
    bool UpdateInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors);

    std::unique_ptr<VulkanBuffer> m_instanceBuffer;
    VulkanContext* m_context;
    std::vector<std::shared_ptr<VulkanRenderObject>> m_renderObjects;
    std::unordered_map<AActor*, std::vector<UMeshComponent*>> m_actorRenderData;
    std::unordered_map<UMeshComponent*, std::shared_ptr<VulkanRenderObject>> m_meshToRenderObject;
    std::vector<AActor*> m_previousFrameActors;

    bool m_initialized = false;
};