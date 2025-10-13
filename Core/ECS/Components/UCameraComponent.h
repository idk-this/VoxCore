//
// Created by IDKTHIS on 15.09.2025.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include "Core/ECS/Components/UBaseComponent.h"
#include "Core/Export.h"

class UTransformComponent;

class VOXCORE_API UCameraComponent : public UBaseComponent
{
public:
    UCLASS(UCameraComponent);

    glm::vec3 RelativeTransform = glm::vec3(0.0f);              // Локальное смещение камеры от родителя
    glm::vec3 RelativeRotation = glm::vec3(0.0f);               // Pitch (X), Yaw (Y), Roll (Z) в градусах

    float fov = 90.0f;
    float aspect = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    void AddYawPitch(float yawOffset, float pitchOffset) {
        RelativeRotation.y += yawOffset;
        RelativeRotation.x += pitchOffset;

        RelativeRotation.y = glm::mod(RelativeRotation.y + 180.0f, 360.0f) - 180.0f;
        RelativeRotation.x = glm::clamp(RelativeRotation.x, -89.0f, 89.0f); // Ограничиваем pitch для предотвращения flip
    }

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetProjectionMatrix() const;
    [[nodiscard]] glm::vec3 GetWorldPosition() const;
    [[nodiscard]] glm::vec3 GetForwardVector() const;
    [[nodiscard]] glm::vec3 GetRightVector() const;
    [[nodiscard]] glm::vec3 GetUpVector() const;
};