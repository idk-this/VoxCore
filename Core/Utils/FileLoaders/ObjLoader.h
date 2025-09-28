//
// Created by IDKTHIS on 28.09.2025.
//

#pragma once
#include <string>


class UMesh;

namespace Engine
{
    namespace FileLoaders
    {
        class ObjLoader {
            static bool Load(const std::string& filename, UMesh& mesh);
        };
    }
}

