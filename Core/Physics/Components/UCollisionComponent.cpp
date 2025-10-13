//
// Created by IDKTHIS on 13.10.2025.
//

#include "UCollisionComponent.h"

void UCollisionComponent::AddBoxCollision(const glm::vec3& center, const glm::vec3& extents)
{
    m_collisionBoxes.push_back({center, extents});
}

void UCollisionComponent::ClearCollisionShapes()
{
    m_collisionBoxes.clear();
}

void UCollisionComponent::UpdateBoundingBox()
{
    if (m_collisionBoxes.empty()) {
        m_boundingBox = FAABB{glm::vec3(0.0f), glm::vec3(0.0f)};
        return;
    }

    glm::vec3 minBounds = m_collisionBoxes[0].center - m_collisionBoxes[0].extents;
    glm::vec3 maxBounds = m_collisionBoxes[0].center + m_collisionBoxes[0].extents;

    for (const auto& box : m_collisionBoxes) {
        glm::vec3 boxMin = box.center - box.extents;
        glm::vec3 boxMax = box.center + box.extents;

        minBounds = glm::min(minBounds, boxMin);
        maxBounds = glm::max(maxBounds, boxMax);
    }

    m_boundingBox = FAABB{minBounds, maxBounds};
}
