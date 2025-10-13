//
// Created by IDKTHIS on 13.10.2025.
//

#include "UPhysicComponent.h"

#include "Core/ECS/Base/AActor.h"

void UPhysicComponent::Update(float deltaTime)
{
    UBaseComponent::Update(deltaTime);
    if (m_useGravity && !m_isGrounded)
    {
        m_forceAccumulator += glm::vec3(0.0f, -9.81f, 0.0f) * m_mass;
    }

    if (m_mass != 0.0f)
    {
        m_acceleration = m_forceAccumulator / m_mass;
    }
    if (m_isGrounded) {
        m_velocity.x *= m_friction;
        m_velocity.z *= m_friction;
        m_velocity.y = 0.0f;
    }

    m_velocity += m_acceleration * deltaTime;

    if (AActor* owner = GetOwner())
    {
        glm::vec3 newPosition = owner->GetComponent<UTransformComponent>()->position + m_velocity * deltaTime;
        owner->GetComponent<UTransformComponent>()->position = newPosition;
    }
    m_forceAccumulator = glm::vec3(0.0f);
    m_isGrounded = false;
}
