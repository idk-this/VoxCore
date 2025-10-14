//
// Created by IDKTHIS on 15.09.2025.
//

#include "UCameraComponent.h"

#include "UTransformComponent.h"
#include "Core/ECS/Base/AActor.h"

glm::quat UCameraComponent::GetOrientation() const
{
    // Правильный порядок для FPS камеры: Yaw (вокруг Y), затем Pitch (вокруг X)
    // Roll игнорируем для FPS камеры
    return glm::quat(glm::vec3(
        glm::radians(RelativeRotation.x), // Pitch (X)
        glm::radians(RelativeRotation.y), // Yaw (Y)
        0.0f // Roll (Z) = 0
    ));
}

glm::vec3 UCameraComponent::GetForwardVector() const
{
    // Вектор "вперед" в мировом пространстве
    glm::quat orientation = GetOrientation();
    return orientation * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 UCameraComponent::GetRightVector() const
{
    // Вектор "вправо" в мировом пространстве
    glm::quat orientation = GetOrientation();
    return orientation * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 UCameraComponent::GetUpVector() const
{
    // Вектор "вверх" в мировом пространстве
    glm::quat orientation = GetOrientation();
    return orientation * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 UCameraComponent::GetViewMatrix() const
{
    glm::vec3 worldPos = GetWorldPosition();
    glm::vec3 forward = GetForwardVector();
    glm::vec3 up = GetUpVector();

    return glm::lookAt(worldPos, worldPos + forward, up);
}

glm::mat4 UCameraComponent::GetProjectionMatrix() const
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    proj[1][1] *= -1; // Инвертируем Y для Vulkan/OpenGL
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