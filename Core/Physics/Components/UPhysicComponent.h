//
// Created by IDKTHIS on 13.10.2025.
//

#pragma once
#include <glm/glm.hpp>

#include "Core/ECS/Components/UBaseComponent.h"


class VOXCORE_API UPhysicComponent : public UBaseComponent {
    UCLASS(UPhysicComponent);
public:
    UPhysicComponent() = default;

    void Update(float deltaTime) override;

    void SetMass(float mass) { m_mass = mass; }
    void SetVelocity(const glm::vec3& velocity) { m_velocity = velocity; }
    void SetAcceleration(const glm::vec3& acceleration) { m_acceleration = acceleration; }
    void SetUseGravity(bool useGravity) { m_useGravity = useGravity; }

    glm::vec3 GetVelocity() const { return m_velocity; }
    glm::vec3 GetAcceleration() const { return m_acceleration; }
    float GetMass() const { return m_mass; }
    bool GetUseGravity() const { return m_useGravity; }
    void AddForce(const glm::vec3& force) { m_forceAccumulator += force; }
    void SetGrounded(bool grounded) { m_isGrounded = grounded; }
    bool IsGrounded() const { return m_isGrounded; }

    void SetFriction(float friction) { m_friction = friction; }
    float GetFriction() const { return m_friction; }

private:
    float m_mass = 1.0f;
    glm::vec3 m_velocity{0.0f};
    glm::vec3 m_acceleration{0.0f};
    bool m_isGrounded = false;
    float m_friction = 0.8f;
    bool m_useGravity = true;
    glm::vec3 m_forceAccumulator{0.0f};
};