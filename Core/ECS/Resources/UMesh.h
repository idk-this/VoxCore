//
// Created by IDKTHIS on 25.09.2025.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <random>

#include "Core/ECS/Base/UObject.h"

class UMesh : public UObject {
    UCLASS(UMesh);
public:
    UMesh() = default;

    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec2> texCoords;
    void SetCubeMesh() {
        float size = 1.0f;

        vertices = {
            // -Z (back)
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },

            // +Z (front)
            { -0.5f, -0.5f,  0.5f },
            {  0.5f, -0.5f,  0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f },

            // -Y (bottom)
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f,  0.5f },
            { -0.5f, -0.5f,  0.5f },

            // +Y (top)
            { -0.5f,  0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f },

            // -X (left)
            { -0.5f, -0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f,  0.5f },
            { -0.5f, -0.5f,  0.5f },

            // +X (right)
            { 0.5f, -0.5f, -0.5f },
            { 0.5f,  0.5f, -0.5f },
            { 0.5f,  0.5f,  0.5f },
            { 0.5f, -0.5f,  0.5f },
        };

        for (auto& v : vertices) v *= size;

        indices = {
            0,1,2, 0,2,3,       // -Z
            4,5,6, 4,6,7,       // +Z
            8,9,10, 8,10,11,    // -Y
            12,13,14, 12,14,15, // +Y
            16,17,18, 16,18,19, // -X
            20,21,22, 20,22,23  // +X
        };

        texCoords = {
            // -Z
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
            // +Z
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
            // -Y
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
            // +Y
            {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
            // -X
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
            // +X
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
        };

      //  GeneratePerFaceColors();
    }
    void Clear() {
        vertices.clear();
        indices.clear();
        colors.clear();
        texCoords.clear();
    }
};