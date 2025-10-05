//
// Created by IDKTHIS on 15.09.2025.
//

#include "UCameraComponent.h"

#include "UTransformComponent.h"
#include "Core/ECS/Base/AActor.h"

glm::vec3 UCameraComponent::GetForwardVector() const
{
    float pitch = glm::radians(RelativeRotation.x);
    float yaw   = glm::radians(RelativeRotation.y);

    glm::vec3 forward;
    forward.x = cos(pitch) * cos(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * sin(yaw);

    return glm::normalize(forward);
}

glm::mat4 UCameraComponent::GetViewMatrix() const
{
    glm::vec3 worldPos = GetWorldPosition();

    glm::quat qYaw   = glm::angleAxis(glm::radians(-RelativeRotation.y), glm::vec3(0, 1, 0));
    glm::quat qPitch = glm::angleAxis(glm::radians(-RelativeRotation.x), glm::vec3(1, 0, 0));
    glm::quat qRoll  = glm::angleAxis(glm::radians(RelativeRotation.z), glm::vec3(0, 0, 1));

    glm::quat orientation = qYaw * qPitch * qRoll;
    auto* transform = m_owner->GetComponent<UTransformComponent>();
    glm::vec3 front = transform ? transform->GetForwardVector() : glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = transform ? transform->GetUpVector() : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 forward = orientation * glm::vec3(0, 0, 1);
    //glm::vec3 up      = orientation * glm::vec3(0, 1, 0);

    return glm::lookAt(worldPos, worldPos + front, up);
}

glm::mat4 UCameraComponent::GetProjectionMatrix() const
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    proj[1][1] *= -1;
    return proj;
}

glm::vec3 UCameraComponent::GetWorldPosition() const
{
    auto* transform = m_owner->GetComponent<UTransformComponent>();
    if (!transform)
        return RelativeTransform;

    glm::vec3 basePos = transform->position;
    glm::quat baseRot = transform->rotation;

    return basePos + (baseRot * RelativeTransform);
}
