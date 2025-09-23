//
// Created by IDKTHIS on 15.09.2025.
//

#include "UCameraComponent.h"

#include "UTransformComponent.h"
#include "Core/ECS/Base/AActor.h"


glm::mat4 UCameraComponent::GetViewMatrix() const
{
    glm::vec3 worldPos = GetWorldPosition();
    auto* transform = m_owner->GetComponent<UTransformComponent>();
    glm::vec3 front = transform ? transform->GetForwardVector() : glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = transform ? transform->GetUpVector() : glm::vec3(0.0f, 1.0f, 0.0f);
    return glm::lookAt(worldPos, worldPos + front, up);
}

glm::mat4 UCameraComponent::GetProjectionMatrix() const
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);

    // Vulkan требует Y-флип
    proj[1][1] *= -1;

    return proj;
}

glm::vec3 UCameraComponent::GetWorldPosition() const
{
    auto* transform = m_owner->GetComponent<UTransformComponent>();
    glm::vec3 basePos = transform->position;
    glm::quat baseRot = transform->rotation;
    return basePos + (baseRot * RelativeTransform);
}
