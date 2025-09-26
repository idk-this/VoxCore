//
// Created by IDKTHIS on 25.09.2025.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>

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


    void SetIcosahedronMesh() {
        const float X = 0.525731112119133606f;
        const float Z = 0.850650808352039932f;

        std::vector<glm::vec3> baseVertices = {
            {-X, 0.0f, Z}, {X, 0.0f, Z}, {-X, 0.0f, -Z}, {X, 0.0f, -Z},
            {0.0f, Z, X}, {0.0f, Z, -X}, {0.0f, -Z, X}, {0.0f, -Z, -X},
            {Z, X, 0.0f}, {-Z, X, 0.0f}, {Z, -X, 0.0f}, {-Z, -X, 0.0f}
        };

        std::vector<uint32_t> baseIndices = {
            0,4,1,  0,9,4,  9,5,4,  4,5,8,  4,8,1,
            8,10,1, 8,3,10, 5,3,8,  5,2,3,  2,7,3,
            7,10,3, 7,6,10, 7,11,6, 11,0,6, 0,1,6,
            6,1,10, 9,0,11, 9,11,2, 9,2,5,  7,2,11
        };

        vertices.clear();
        indices.clear();
        colors.clear();

        // Каждая грань свой цвет
        for (size_t f = 0; f < baseIndices.size(); f += 3) {
            glm::vec3 c = GenerateRandomColor();
            for (int i = 0; i < 3; i++) {
                uint32_t vi = baseIndices[f + i];
                vertices.push_back(baseVertices[vi]);
                colors.push_back(c);
                indices.push_back(static_cast<uint32_t>(indices.size()));
            }
        }
    }

   bool LoadFromOBJ(const std::string& filename) {
    vertices.clear();
    indices.clear();
    colors.clear();
    texCoords.clear();

    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        return false;
    }

    size_t vertexCount = 0;
    size_t texCoordCount = 0;
    size_t faceCount = 0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.rfind("v ", 0) == 0) vertexCount++;
        else if (line.rfind("vt ", 0) == 0) texCoordCount++;
        else if (line.rfind("f ", 0) == 0) faceCount++;
    }
    file.clear();
    file.seekg(0);

    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempTexCoords;

    tempVertices.reserve(vertexCount);
    tempTexCoords.reserve(texCoordCount);
    vertices.reserve(faceCount * 3);
    colors.reserve(faceCount * 3);
    texCoords.reserve(faceCount * 3);
    indices.reserve(faceCount * 3);

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("v ", 0) == 0) {
            glm::vec3 vertex;
            if (sscanf(line.c_str() + 2, "%f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
                tempVertices.push_back(vertex);
            }
        }
        else if (line.rfind("vt ", 0) == 0) {
            glm::vec2 uv;
            if (sscanf(line.c_str() + 3, "%f %f", &uv.x, &uv.y) == 2) {
                uv.y = 1.0f - uv.y; // инвертируем Y (OpenGL-style)
                tempTexCoords.push_back(uv);
            }
        }
        else if (line.rfind("f ", 0) == 0) {
            std::vector<uint32_t> vIdx, tIdx;
            const char* ptr = line.c_str() + 2;

            while (*ptr) {
                while (*ptr == ' ') ptr++;
                if (!*ptr) break;

                uint32_t vIndex = 0, tIndex = 0;
                // vIndex
                while (*ptr >= '0' && *ptr <= '9') {
                    vIndex = vIndex * 10 + (*ptr - '0');
                    ptr++;
                }
                if (*ptr == '/') {
                    ptr++;
                    if (*ptr >= '0' && *ptr <= '9') {
                        while (*ptr >= '0' && *ptr <= '9') {
                            tIndex = tIndex * 10 + (*ptr - '0');
                            ptr++;
                        }
                    }
                    // нормали можно тоже парсить, но пока пропускаем
                    while (*ptr && *ptr != ' ') ptr++;
                }

                if (vIndex > 0) vIdx.push_back(vIndex - 1);
                if (tIndex > 0) tIdx.push_back(tIndex - 1);
            }

            if (vIdx.size() >= 3) {
                //glm::vec3 faceColor = GenerateRandomColor();
                uint32_t baseIndex = (uint32_t)vertices.size();

                for (size_t i = 0; i < vIdx.size(); i++) {
                    vertices.push_back(tempVertices[vIdx[i]]);
                    //colors.push_back(faceColor);
                    if (i < tIdx.size() && tIdx[i] < tempTexCoords.size())
                        texCoords.push_back(tempTexCoords[tIdx[i]]);
                    else
                        texCoords.push_back(glm::vec2(0.0f, 0.0f));
                }

                // триангуляция
                for (size_t i = 1; i + 1 < vIdx.size(); i++) {
                    indices.push_back(baseIndex);
                    indices.push_back(baseIndex + (uint32_t)i);
                    indices.push_back(baseIndex + (uint32_t)i + 1);
                }
            }
        }
    }
    file.close();

    return !vertices.empty();
}

    void Clear() {
        vertices.clear();
        indices.clear();
        colors.clear();
        texCoords.clear();
    }
private:
    glm::vec3 GenerateRandomColor() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        return glm::vec3(dis(gen), dis(gen), dis(gen));
    }

    void GeneratePerFaceColors() {
        colors.clear();

        if (indices.empty() || vertices.empty()) return;

        // Generate a unique color for each face
        for (size_t i = 0; i < indices.size(); i += 3) {
            glm::vec3 faceColor = GenerateRandomColor();

            // Assign the same color to all vertices of the face
            // This creates flat shading effect
            if (i < vertices.size()) {
                colors.push_back(faceColor);
                if (i + 1 < vertices.size()) colors.push_back(faceColor);
                if (i + 2 < vertices.size()) colors.push_back(faceColor);
            }
        }

        // Ensure we have exactly one color per vertex
        if (colors.size() != vertices.size()) {
            colors.resize(vertices.size(), glm::vec3(1.0f, 1.0f, 1.0f));
        }
    }
};