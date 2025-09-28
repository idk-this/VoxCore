//
// Created by IDKTHIS on 28.09.2025.
//

#pragma once
#include <limits>
#include <memory>
#include <glm/glm.hpp>


class AActor;

struct FHitResult {
    bool bHit = false;
    std::shared_ptr<AActor> HitActor = nullptr;
    glm::vec3 HitLocation{};
    float Distance = std::numeric_limits<float>::max();
};
