//
// Created by IDKTHIS on 25.07.2025.
//

#pragma once

struct InstanceData {
    glm::mat4 model;
    glm::vec3 color;
};

class IVertexLayout {
public:
    virtual ~IVertexLayout() = default;

    virtual void* getNativeBindingDescription() const = 0;
    virtual void* getNativeAttributeDescriptions() const = 0;
};