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

        bindingDescs[0].binding = 0;
        bindingDescs[0].stride = sizeof(Vertex);
        bindingDescs[0].inputRate = vk::VertexInputRate::eVertex;

        // --- Instance Buffer ---
        bindingDescs[1].binding = 1;
        bindingDescs[1].stride = sizeof(InstanceData);
        bindingDescs[1].inputRate = vk::VertexInputRate::eInstance;

        // --- Vertex attributes ---
        attributeDescs.resize(8); // 2 для вершин + 6 для инстансов

        // Vertex
        attributeDescs[0] = { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) };
        attributeDescs[1] = { 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) };

        // Instance matrix (mat4 = 4 vec4)
        attributeDescs[2] = { 2, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 0 };
        attributeDescs[3] = { 3, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 1 };
        attributeDescs[4] = { 4, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 2 };
        attributeDescs[5] = { 5, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(InstanceData, model) + sizeof(glm::vec4) * 3 };

        // Instance color
        attributeDescs[6] = { 6, 1, vk::Format::eR32G32B32Sfloat, offsetof(InstanceData, color) };
        attributeDescs[7] = { 7, 1, vk::Format::eR32Sfloat, offsetof(InstanceData, padding) }; // padding для выравнивания
    }

    void* getNativeBindingDescription() const override {
        return (void*)bindingDescs.data();
    }

    void* getNativeAttributeDescriptions() const override {
        return (void*)attributeDescs.data();
    }
};
