//
// Created by IDKTHIS on 05.09.2025.
//

#pragma once
#include <glm/glm.hpp>

struct CameraData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 position;
};

class ICameraUBO {
public:
    virtual ~ICameraUBO() = default;
    virtual void Update(const CameraData& data) = 0;
    virtual void* GetHandle() const = 0;
};