//
// Created by IDKTHIS on 05.09.2025.
//

#pragma once
#include <glm/glm.hpp>

struct CameraData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 pos;
    float padding;
};

class  ICameraUBO {
public:
    virtual ~ICameraUBO() = default;
    virtual void Update(void* cmd, const CameraData& data) = 0;
};