//
// Created by IDKTHIS on 17.07.2025.
//

#pragma once
#include "Core/ECS/Components/UBaseComponent.h""
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


class UTransformComponent : public UBaseComponent {
public:
    UCLASS(UTransformComponent);
    UTransformComponent()
        : position(0.0f), rotation(glm::quat_identity<float, glm::defaultp>()), scale(1.0f) {}

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 GetTransformMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 R = glm::toMat4(rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        return T * R * S;
    }
    void Move(const glm::vec3& delta);
    glm::vec3 GetForwardVector() const
    {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 GetRightVector() const
    {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 GetUpVector() const
    {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    void SetRotationYawPitch(float yaw, float pitch);
    void SetRotationEuler(const glm::vec3& eulerRadians) {
        rotation = glm::quat(eulerRadians);
    }
    void SetPosition(const glm::vec3& newPosition) {
        position = newPosition;
    }
    glm::vec3 GetRotationEuler() const {
        return glm::eulerAngles(rotation);
    }
};
