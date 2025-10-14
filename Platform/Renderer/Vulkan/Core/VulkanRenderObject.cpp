//
// Created by IDKTHIS on 29.09.2025.
//

#include "VulkanRenderObject.h"

#include "Core/ECS/Base/AActor.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Common/VulkanBuffer.h"
#include "Platform/Renderer/Vulkan/Common/VulkanTexture.h"


VulkanRenderObject::VulkanRenderObject(VulkanContext* context, UMeshComponent* mesh)
    : m_context(context), m_mesh(mesh) {}

VulkanRenderObject::~VulkanRenderObject() {
    Cleanup();
}

bool VulkanRenderObject::Initialize() {
    if (!m_mesh || m_mesh->Mesh->vertices.empty()) return false;

    try {
        if (!CreateBuffers(m_mesh->Mesh->vertices, m_mesh->Mesh->indices, m_mesh->Mesh->colors, m_mesh->Mesh->texCoords, m_mesh->Mesh->normals))
            return false;

        if (m_mesh->Texture) {
            if (!CreateTextureResources(m_mesh->Texture.get())) {
                LOG_WARN("Vulkan", "Failed to create texture resources for mesh");
            }
        }

        m_indexCount = static_cast<uint32_t>(m_mesh->Mesh->indices.size());
        m_initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Vulkan", "Failed to initialize VulkanRenderObject: {}", e.what());
        Cleanup();
        return false;
    }
}

bool VulkanRenderObject::CreateBuffers(const std::vector<glm::vec3>& vertices,
                                       const std::vector<uint32_t>& indices,
                                       const std::vector<glm::vec3>& colors,
                                       const std::vector<glm::vec2>& texCoords,
                                       const std::vector<glm::vec3>& normals) {
    try {
        LOG_DEBUG("Vulkan", "Initializing VulkanRenderObject for mesh, vertices={}, indices={}",
          vertices.size(), indices.size());
        std::vector<Vertex> vertexData;
        vertexData.reserve(vertices.size());

        for (size_t i = 0; i < vertices.size(); ++i) {
            glm::vec3 color = (i < colors.size()) ? colors[i] : glm::vec3(1.0f);
            glm::vec2 uv = (i < texCoords.size()) ? texCoords[i] : glm::vec2(0.0f);
            glm::vec3 normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f, 1.0f, 0.0f);
            vertexData.push_back(Vertex{ vertices[i], color, uv, normal});
        }

        // Vertex buffer
        m_vertexBuffer = std::make_unique<VulkanBuffer>(m_context);
        m_vertexBuffer->Create(sizeof(Vertex) * vertexData.size(), vk::BufferUsageFlagBits::eVertexBuffer);
        m_vertexBuffer->UpdateBufferDataArray(vertexData);

        // Index buffer
        m_indexBuffer = std::make_unique<VulkanBuffer>(m_context);
        m_indexBuffer->Create(sizeof(uint32_t) * indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);
        m_indexBuffer->UpdateBufferDataArray(indices);



        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Vulkan", "Failed to create buffers: {}", e.what());
        Cleanup();
        return false;
    }
}

bool VulkanRenderObject::CreateTextureResources(UTexture* texture) {
    try {
        m_context->logicalDevice->GetHandle().waitIdle();

        m_texture = std::make_unique<VulkanTexture>(m_context);
        m_texture->UploadFromCPU(texture);

        m_context->logicalDevice->GetHandle().waitIdle();

        auto layout = m_context->pipelines[PipelineType::Graphics].descriptorSetLayouts[1];
        if (!layout) {
            LOG_ERROR("Vulkan", "Texture descriptor set layout not found");
            return false;
        }

        vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 1);
        vk::DescriptorPoolCreateInfo poolInfo({}, 1, 1, &poolSize);
        m_textureDescriptorPool = m_context->logicalDevice->GetHandle().createDescriptorPool(poolInfo);

        vk::DescriptorSetAllocateInfo allocInfo(m_textureDescriptorPool, 1, &layout);
        auto descriptorSets = m_context->logicalDevice->GetHandle().allocateDescriptorSets(allocInfo);
        m_textureDescriptorSet = descriptorSets.front();

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = m_texture->GetImageView();
        imageInfo.sampler = m_texture->GetSampler();

        vk::WriteDescriptorSet descriptorWrite{};
        descriptorWrite.dstSet = m_textureDescriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        m_context->logicalDevice->GetHandle().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

        LOG_INFO("Texture", "Successfully created texture resources!");
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Vulkan", "Failed to create texture resources: {}", e.what());
        return false;
    }
}

bool VulkanRenderObject::Draw(const std::any& drawInfo) {
    if (!m_initialized) return false;

    try {
        auto info = std::any_cast<VulkanDrawCallInfo>(drawInfo);

        // Привязываем индексный буфер
        info.commandContext.bindIndexBuffer(m_indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);

        // Привязываем дескрипторные наборы
        if (m_textureDescriptorSet) {
            std::array<vk::DescriptorSet, 2> descriptorSets = {
                info.cameraData,  // set = 0
                m_textureDescriptorSet  // set = 1
            };
            info.commandContext.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                m_context->pipelines[PipelineType::Graphics].layout,
                0,  // firstSet
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data(),
                0, nullptr
            );
        } else {
            // Если текстуры нет, привязываем только камеру
            info.commandContext.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                m_context->pipelines[PipelineType::Graphics].layout,
                0,  // firstSet
                1,
                &info.cameraData,
                0, nullptr
            );
        }

        info.commandContext.drawIndexed(m_indexCount, 1, 0, 0, 0);
        return true;
    }
    catch (const std::bad_any_cast& e) {
        LOG_ERROR("Vulkan", "Failed to cast draw info: {}", e.what());
        return false;
    }
}

void VulkanRenderObject::Cleanup() {
    return;
    if (m_vertexBuffer) { m_vertexBuffer->Destroy(); m_vertexBuffer.reset(); }
    if (m_indexBuffer) { m_indexBuffer->Destroy(); m_indexBuffer.reset(); }
    if (m_texture) { m_texture.reset(); }
    if (m_textureDescriptorPool) {
        m_context->logicalDevice->GetHandle().destroyDescriptorPool(m_textureDescriptorPool);
        m_textureDescriptorPool = nullptr;
    }
    m_textureDescriptorSet = nullptr;
    m_initialized = false;
}

void VulkanRenderObject::UpdateMeshData(UMeshComponent* mesh)
{
     if (!mesh || !mesh->Mesh) return;

    if (mesh->Mesh->meshDirty) {
        try {
            LOG_DEBUG("Vulkan", "Updating mesh buffers for '{}'", mesh->GetOwner()->GetName());

            if (m_vertexBuffer) { m_vertexBuffer->Destroy(); m_vertexBuffer.reset(); }
            if (m_indexBuffer)  { m_indexBuffer->Destroy();  m_indexBuffer.reset();  }

            if (!CreateBuffers(mesh->Mesh->vertices, mesh->Mesh->indices, mesh->Mesh->colors, mesh->Mesh->texCoords, mesh->Mesh->normals)) {
                LOG_ERROR("Vulkan", "Failed to recreate vertex/index buffers for mesh '{}'", mesh->GetOwner()->GetName());
            } else {
                m_indexCount = static_cast<uint32_t>(mesh->Mesh->indices.size());
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Vulkan", "Exception while updating mesh buffers: {}", e.what());
        }
        mesh->Mesh->meshDirty = false;
    }

    /*// 2️⃣ Проверяем обновление текстуры
    if (mesh->textureDirty) {
        try {
            LOG_DEBUG("Vulkan", "Updating texture for '{}'", mesh->GetOwner()->GetName());

            // Удаляем старые дескрипторы и текстуру
            if (m_texture) m_texture.reset();
            if (m_textureDescriptorPool) {
                m_context->logicalDevice->GetHandle().destroyDescriptorPool(m_textureDescriptorPool);
                m_textureDescriptorPool = nullptr;
            }
            m_textureDescriptorSet = nullptr;

            if (mesh->Texture) {
                if (!CreateTextureResources(mesh->Texture.get())) {
                    LOG_WARN("Vulkan", "Failed to recreate texture resources for mesh '{}'", mesh->GetOwner()->GetName());
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Vulkan", "Exception while updating texture: {}", e.what());
        }

        // Сбрасываем флаг
        mesh->textureDirty = false;
    }*/
}
