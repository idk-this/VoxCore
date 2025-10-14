//
// Created by IDKTHIS on 25.07.2025.
//

#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;

};
struct LightData {
    glm::vec3 position;
    alignas(16) glm::vec3 color;
    glm::vec3 ambient;
    alignas(16) float intensity;
    float specularPower;
    alignas(16) glm::vec3 padding;
};