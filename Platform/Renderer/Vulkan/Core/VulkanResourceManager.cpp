//
// Created by IDKTHIS on 29.09.2025.
//

#include "VulkanResourceManager.h"

#include "Core/ECS/Base/UWorld.h"
#include "Core/Log/Logger.h"
#include "glm/gtx/string_cast.hpp"
VulkanResourceManager::VulkanResourceManager(VulkanContext* context)
    : m_context(context)
{
}

VulkanResourceManager::~VulkanResourceManager() {
    VulkanResourceManager::Cleanup();
}

bool VulkanResourceManager::Initialize() {
    if (m_initialized) return true;
    m_initialized = true;
    LOG_INFO("Vulkan", "VulkanResourceManager initialized");
    return true;
}

void VulkanResourceManager::Cleanup() {
    if (!m_initialized) return;

    for (auto& renderObject : m_renderObjects) {
        if (renderObject) {
            renderObject->Cleanup();
        }
    }

    m_renderObjects.clear();
    m_actorRenderData.clear();
    m_meshToRenderObject.clear();
    m_previousFrameActors.clear();

    m_initialized = false;
    LOG_INFO("Vulkan", "VulkanResourceManager cleaned up");
}

void VulkanResourceManager::UpdateWorldState(UWorld* world) {
    if (!world || !m_initialized) return;
    auto currentActors = world->GetActors();

    for (auto& actor : currentActors) {
        if (!actor) continue;

        bool isNewActor = true;
        for (auto& prevActor : m_previousFrameActors) {
            if (prevActor == actor.get()) {
                isNewActor = false;
                break;
            }
        }

        if (isNewActor) {
            OnActorSpawn(actor.get());
        }
    }

    for (auto& prevActor : m_previousFrameActors) {
        if (!prevActor) continue;

        bool actorStillExists = false;
        for (auto& currentActor : currentActors) {
            if (currentActor.get() == prevActor) {
                actorStillExists = true;
                break;
            }
        }

        if (!actorStillExists) {
            OnActorDestroy(prevActor);
        }
    }
    for (auto& actor : currentActors) {
        if (!actor) continue;

        auto it = m_actorRenderData.find(actor.get());
        if (it != m_actorRenderData.end()) {
            for (auto* meshComponent : it->second) {
                if (meshComponent && meshComponent->Mesh->meshDirty) {
                    UpdateMeshComponent(meshComponent);
                    meshComponent->Mesh->meshDirty = false;
                }
            }
        }
    }
    m_previousFrameActors.clear();
    for (auto& actor : currentActors) {
        if (actor) {
            m_previousFrameActors.push_back(actor.get());
        }
    }
}

void VulkanResourceManager::PrepareInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors)
{
    if (!mesh || mesh->Mesh->vertices.empty()) return;

    if (!m_instanceBuffer)
        m_instanceBuffer = std::make_unique<VulkanBuffer>(m_context);

    std::vector<InstanceData> instances;
    for (auto* actor : actors) {
        InstanceData inst{};
        auto transform = actor->GetComponent<UTransformComponent>();
        inst.model = transform ? transform->GetTransformMatrix() : glm::mat4(1.0f);
        inst.color = glm::vec3(1.0f);
        instances.push_back(inst);
    }

    if (instances.empty()) return;

    m_instanceBuffer->Create(sizeof(InstanceData) * instances.size(), vk::BufferUsageFlagBits::eVertexBuffer);
    m_instanceBuffer->UpdateBufferDataArray(instances);
}

bool VulkanResourceManager::UpdateInstanceBuffer(UMeshComponent* mesh, const std::vector<AActor*>& actors)
{
    if (!m_instanceBuffer) m_instanceBuffer = std::make_unique<VulkanBuffer>(m_context);

    std::vector<InstanceData> instances;
    for (auto* actor : actors) {
        InstanceData inst{};
        auto transform = actor->GetComponent<UTransformComponent>();
        inst.model = transform ? transform->GetTransformMatrix() : glm::mat4(1.0f);
        inst.color = glm::vec3(1.0f);
        instances.push_back(inst);
    }

    if (instances.empty()) return false;

    m_instanceBuffer->Create(sizeof(InstanceData) * instances.size(), vk::BufferUsageFlagBits::eVertexBuffer);
    m_instanceBuffer->UpdateBufferDataArray(instances);
    return true;
}

void VulkanResourceManager::RenderObjects(vk::CommandBuffer& cmd, VulkanCameraUBO* cameraUBO, uint32_t currentFrame)
{
    for (auto& renderObject : m_renderObjects) {
        if (!renderObject || !renderObject->IsValid()) continue;

        AActor* actor = renderObject->m_mesh->GetOwner();
        if (!actor) continue;

        if (UpdateInstanceBuffer(renderObject->m_mesh, {actor})) {
            // ПРИВЯЗЫВАЕМ ВЕРШИННЫЕ БУФЕРЫ
            std::vector<vk::Buffer> vertexBuffers;
            std::vector<vk::DeviceSize> offsets;

            // Вершинный буфер (binding 0)
            vertexBuffers.push_back(renderObject->GetVertexBuffer()->GetBuffer());
            offsets.push_back(0);

            // Инстансный буфер (binding 1)
            vertexBuffers.push_back(m_instanceBuffer->GetBuffer());
            offsets.push_back(0);

            cmd.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());

            VulkanDrawCallInfo drawCallInfo{
                cmd,
                cameraUBO->GetDescriptorSet(),
                currentFrame
            };

            renderObject->Draw(drawCallInfo);
        }
    }
}

void VulkanResourceManager::OnActorSpawn(AActor* actor) {
    if (!actor) return;

    LOG_DEBUG("Vulkan", "Actor spawned: {}", actor->GetName());

    auto meshComponents = { actor->GetComponent<UMeshComponent>() };
    std::vector<UMeshComponent*> actorData;

    for (auto* meshComponent : meshComponents) {
        if (meshComponent) {
            ProcessMeshComponent(actor, meshComponent);
            actorData.push_back(meshComponent);
        }
    }

    m_actorRenderData[actor] = std::move(actorData);
}

void VulkanResourceManager::OnActorDestroy(AActor* actor) {
    if (!actor) return;

   // LOG_DEBUG("Vulkan", "Actor destroyed: {}", actor->GetName());

    auto it = m_actorRenderData.find(actor);
    if (it != m_actorRenderData.end()) {
        for (auto* meshComponent : it->second) {
            RemoveMeshComponent(meshComponent);
        }
        m_actorRenderData.erase(it);
    }
}

void VulkanResourceManager::ProcessMeshComponent(AActor* actor, UMeshComponent* mesh) {
    if (!mesh || !mesh->Mesh || mesh->Mesh->vertices.empty()) return;

    if (m_meshToRenderObject.find(mesh) != m_meshToRenderObject.end()) {
        return;
    }
    try {
        auto renderObject = std::make_shared<VulkanRenderObject>(m_context, mesh);
        if (renderObject->Initialize()) {
            m_renderObjects.push_back(renderObject);
            m_meshToRenderObject[mesh] = renderObject;
            LOG_DEBUG("Vulkan", "Created render object for mesh component");
        } else {
            LOG_ERROR("Vulkan", "Failed to initialize render object for mesh component");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Vulkan", "Exception while creating render object: {}", e.what());
    }
}

void VulkanResourceManager::RemoveMeshComponent(UMeshComponent* mesh) {
    if (!mesh) return;

    auto it = m_meshToRenderObject.find(mesh);
    if (it != m_meshToRenderObject.end()) {
        auto renderObject = it->second;
        for (auto listIt = m_renderObjects.begin(); listIt != m_renderObjects.end(); ++listIt) {
            if (*listIt == renderObject) {
                (*listIt)->Cleanup();
                m_renderObjects.erase(listIt);
                break;
            }
        }

        m_meshToRenderObject.erase(it);
        LOG_DEBUG("Vulkan", "Removed render object for mesh component");
    }
}

void VulkanResourceManager::UpdateMeshComponent(UMeshComponent* mesh)
{
    if (!mesh) return;

    auto it = m_meshToRenderObject.find(mesh);
    if (it != m_meshToRenderObject.end()) {
        auto& renderObject = it->second;
        if (renderObject) {
            renderObject->UpdateMeshData(mesh);
            LOG_DEBUG("Vulkan", "Updated render object for dirty mesh");
        }
    } else {
        ProcessMeshComponent(mesh->GetOwner(), mesh);
    }
}
