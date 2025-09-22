//
// Created by IDKTHIS on 17.07.2025.
//

#include "UTransformComponent.h"

void UTransformComponent::Move(const glm::vec3& delta)
{
    position += delta;
}

void UTransformComponent::SetRotationYawPitch(float yaw, float pitch)
{
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    rotation = glm::quatLookAt(front, glm::vec3(0.0f, 1.0f, 0.0f));
}
