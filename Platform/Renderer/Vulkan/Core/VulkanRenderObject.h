//
// Created by IDKTHIS on 29.09.2025.
//

#pragma once
#include <vulkan/vulkan.hpp>

#include "Core/Common/Vertex.h"
#include "Platform/Renderer/Common/IRenderObject.h"
#include "Core/ECS/Components/UMeshComponent.h"
#include "Platform/Renderer/Common/IVertexLayout.h"

class UTransformComponent;
class UTexture;
class VulkanContext;
class VulkanBuffer;
class VulkanTexture;

struct VulkanDrawCallInfo {
    vk::CommandBuffer& commandContext;
    vk::DescriptorSet cameraData;
    uint32_t frameIndex;
};

class VulkanRenderObject : public IRenderObject {
public:
    VulkanRenderObject(VulkanContext* context, UMeshComponent* mesh);
    ~VulkanRenderObject() override;

    bool Initialize() override;
    bool Draw(const std::any& drawInfo) override;
    void Cleanup() override;
    void UpdateMeshData(UMeshComponent* mesh);

    [[nodiscard]] uint32_t GetIndexCount() const override { return m_indexCount; }
    [[nodiscard]] bool IsValid() const override { return m_initialized; }

    [[nodiscard]] VulkanBuffer* GetVertexBuffer() { return m_vertexBuffer.get(); }

private:
    bool CreateBuffers(const std::vector<glm::vec3>& vertices,
                       const std::vector<uint32_t>& indices,
                       const std::vector<glm::vec3>& colors,
                       const std::vector<glm::vec2>& texCoords);
    bool CreateTextureResources(UTexture* texture);

public:
    VulkanContext* m_context;
    UMeshComponent* m_mesh;

    std::unique_ptr<VulkanBuffer> m_vertexBuffer;
    std::unique_ptr<VulkanBuffer> m_indexBuffer;

    std::unique_ptr<VulkanTexture> m_texture;

    vk::DescriptorSet m_textureDescriptorSet = nullptr;
    vk::DescriptorPool m_textureDescriptorPool = nullptr;

    uint32_t m_indexCount = 0;
    bool m_initialized = false;
};