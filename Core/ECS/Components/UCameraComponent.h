//
// Created by IDKTHIS on 15.09.2025.
//

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Core/ECS/Components/UBaseComponent.h"
#include "Core/Export.h"


class UTransformComponent;

class VOXCORE_API UCameraComponent : public UBaseComponent
{
public:
    UCLASS(UCameraComponent);

    glm::vec3 RelativeTransform = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat RelativeRotation = glm::quat(glm::vec3(0.0f));
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 90.0f;
    float aspect = 16.0f/9.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    void AddYawPitch(float yawOffset, float pitchOffset) {
        yaw += yawOffset;
        pitch += pitchOffset;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }
    [[nodiscard]] glm::mat4 GetViewMatrix() const;

    [[nodiscard]] glm::mat4 GetProjectionMatrix() const;

    [[nodiscard]] glm::vec3 GetWorldPosition() const;
    [[nodiscard]] glm::vec3 GetForwardVector() const {
        glm::vec3 forward;
        forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward.y = sin(glm::radians(pitch));
        forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::normalize(forward);
    }

};
