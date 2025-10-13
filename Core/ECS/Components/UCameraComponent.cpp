//
// Created by IDKTHIS on 15.09.2025.
//

#include "UCameraComponent.h"

#include "UTransformComponent.h"
#include "Core/ECS/Base/AActor.h"

glm::vec3 UCameraComponent::GetForwardVector() const
{
    // Создаем кватернион вращения из углов Эйлера
    // Порядок: Yaw (Y), Pitch (X), Roll (Z) - но Roll игнорируем для FPS камеры
    glm::quat rotation = glm::quat(glm::vec3(
        glm::radians(RelativeRotation.x), // Pitch
        glm::radians(RelativeRotation.y), // Yaw
        0.0f // Roll = 0 для FPS камеры
    ));

    // Вектор "вперед" в локальном пространстве камеры
    glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    return glm::normalize(forward);
}

glm::vec3 UCameraComponent::GetRightVector() const
{
    // Вектор "вправо" перпендикулярен forward и мировому up
    glm::vec3 forward = GetForwardVector();
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::cross(forward, worldUp);
    return glm::normalize(right);
}

glm::vec3 UCameraComponent::GetUpVector() const
{
    glm::vec3 forward = GetForwardVector();
    glm::vec3 right = GetRightVector();
    glm::vec3 up = glm::cross(right, forward);
    return glm::normalize(up);
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