//
// Created by IDKTHIS on 13.10.2025.
//

#pragma once
#include <glm/glm.hpp>

#include "Core/ECS/Base/AActor.h"
#include "Core/ECS/Components/UBaseComponent.h"


struct FCollisionBox {
    glm::vec3 center;
    glm::vec3 extents;
};

class VOXCORE_API UCollisionComponent : public UBaseComponent {
    UCLASS(UCollisionComponent);
public:
    UCollisionComponent() = default;


    void AddBoxCollision(const glm::vec3& center, const glm::vec3& extents);
    void ClearCollisionShapes();
    void UpdateBoundingBox();

    void SetIsTrigger(bool trigger) { m_isTrigger = trigger; }
    void SetCollisionEnabled(bool enabled) { m_collisionEnabled = enabled; }
    [[nodiscard]] bool IsTrigger() const { return m_isTrigger; }
    [[nodiscard]] bool IsCollisionEnabled() const { return m_collisionEnabled; }

private:
    std::vector<FCollisionBox> m_collisionBoxes;
    FAABB m_boundingBox;
    bool m_collisionEnabled = true;
    bool m_isTrigger = false;
};