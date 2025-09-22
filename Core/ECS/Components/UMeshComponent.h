// Created by IDKTHIS on 17.07.2025
#pragma once
#include "Core/ECS/BaseClasses/UBaseComponent.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>

class UMeshComponent : public UBaseComponent {
public:
    UCLASS(UMeshComponent);
    UMeshComponent() = default;
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> colors;

    void SetCubeMesh() {
        float size = 0.2f + static_cast<float>(std::rand()) / RAND_MAX * 0.8f;

        vertices = {
            // -Z
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },

            // +Z
            { -0.5f, -0.5f,  0.5f },
            {  0.5f, -0.5f,  0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f },

            // -Y
            { -0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f, -0.5f },
            {  0.5f, -0.5f,  0.5f },
            { -0.5f, -0.5f,  0.5f },

            // +Y
            { -0.5f,  0.5f, -0.5f },
            {  0.5f,  0.5f, -0.5f },
            {  0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f },

            // -X
            { -0.5f, -0.5f, -0.5f },
            { -0.5f,  0.5f, -0.5f },
            { -0.5f,  0.5f,  0.5f },
            { -0.5f, -0.5f,  0.5f },

            // +X
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

        auto randColor = []() -> glm::vec3 {
            return glm::vec3(
                static_cast<float>(std::rand()) / RAND_MAX,
                static_cast<float>(std::rand()) / RAND_MAX,
                static_cast<float>(std::rand()) / RAND_MAX
            );
        };

        // По цвету на грань → копируем 4 раза
        for (int face = 0; face < 6; face++) {
            glm::vec3 c = randColor();
            for (int i = 0; i < 4; i++) colors.push_back(c);
        }
    }


};

