//
// Created by IDKTHIS on 25.07.2025.
//

#pragma once
#include "Platform/Renderer/Common/IVertexLayout.h"
#include "Core/Common/Vertex.h"
#include <vulkan/vulkan.hpp>


class VulkanVertexLayout : public IVertexLayout {
public:
    std::vector<vk::VertexInputBindingDescription> bindingDescs;
    std::vector<vk::VertexInputAttributeDescription> attributeDescs;

    VulkanVertexLayout() {
        bindingDescs.resize(2);

        // --- Vertex buffer ---
        bindingDescs[0].binding = 0;
        bindingDescs[0].stride = sizeof(Vertex);
        bindingDescs[0].inputRate = vk::VertexInputRate::eVertex;

        // --- Instance buffer ---
        bindingDescs[1].binding = 1;
        bindingDescs[1].stride = sizeof(InstanceData);
        bindingDescs[1].inputRate = vk::VertexInputRate::eInstance;

        // --- Attributes (ровно как в шейдере) ---
        attributeDescs = {
            // binding 0 → Vertex
            { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) }, // location 0
            { 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)    }, // location 1
            { 2, 0, vk::Format::eR32G32Sfloat,    offsetof(Vertex, texCoord) }, // location 2

            // binding 1 → InstanceData
            { 3, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 0 }, // location 3
            { 4, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 1 }, // location 4
            { 5, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 2 }, // location 5
            { 6, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 3 }, // location 6

            { 7, 1, vk::Format::eR32G32B32Sfloat, offsetof(InstanceData, color) } // location 7
        };
    }

    void* getNativeBindingDescription() const override {
        return (void*)bindingDescs.data();
    }

    void* getNativeAttributeDescriptions() const override {
        return (void*)attributeDescs.data();
    }
};
