//
// Created by IDKTHIS on 28.09.2025.
//

#include "ObjLoader.h"

#include "Core/ECS/Resources/UMesh.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using namespace Engine::FileLoaders;
bool ObjLoader::Load(const std::string& filename, UMesh& mesh)
{
      mesh.Clear();

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
    mesh.vertices.reserve(faceCount * 3);
    mesh.colors.reserve(faceCount * 3);
    mesh.texCoords.reserve(faceCount * 3);
    mesh.indices.reserve(faceCount * 3);

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
                uv.y = 1.0f - uv.y;
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
                    while (*ptr && *ptr != ' ') ptr++;
                }

                if (vIndex > 0) vIdx.push_back(vIndex - 1);
                if (tIndex > 0) tIdx.push_back(tIndex - 1);
            }

            if (vIdx.size() >= 3) {
                uint32_t baseIndex = (uint32_t)mesh.vertices.size();

                for (size_t i = 0; i < vIdx.size(); i++) {
                    mesh.vertices.push_back(tempVertices[vIdx[i]]);
                    if (i < tIdx.size() && tIdx[i] < tempTexCoords.size())
                        mesh.texCoords.push_back(tempTexCoords[tIdx[i]]);
                    else
                        mesh.texCoords.emplace_back(0.0f, 0.0f);
                }

                for (size_t i = 1; i + 1 < vIdx.size(); i++) {
                    mesh.indices.push_back(baseIndex);
                    mesh.indices.push_back(baseIndex + (uint32_t)i);
                    mesh.indices.push_back(baseIndex + (uint32_t)i + 1);
                }
            }
        }
    }
    file.close();

    return !mesh.vertices.empty();
}