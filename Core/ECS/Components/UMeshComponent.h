// Created by IDKTHIS on 17.07.2025
#pragma once
#include "Core/ECS/Components/UBaseComponent.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "Core/ECS/Resources/UTexture.h"
#include "Core/ECS/Resources/UMesh.h"

class UMeshComponent : public UBaseComponent {
public:
    UCLASS(UMeshComponent);
    UMeshComponent() = default;

    std::shared_ptr<UMesh>  Mesh = nullptr;
    std::shared_ptr<UTexture> Texture = nullptr;

};
