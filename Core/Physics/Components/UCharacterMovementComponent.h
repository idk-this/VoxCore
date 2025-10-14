//
// Created by IDKTHIS on 13.10.2025.
//

#pragma once
#include "Core/ECS/Components/UBaseComponent.h"
#include "Core/Physics/Components/UPhysicComponent.h"
#include "Core/Physics/Components/UCollisionComponent.h"
#include "Core/ECS/Base/AActor.h"

class VOXCORE_API UCharacterMovementComponent : public UBaseComponent {
    UCLASS(UCharacterMovementComponent);
public:
    UCharacterMovementComponent() = default;

    void Update(float deltaTime) override;
    bool CheckAABBCollision(const FAABB& a, const FAABB& b, glm::vec3& pushOut);

    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetJumpForce(float force) { m_jumpForce = force; }

private:
    float m_moveSpeed = 5.0f;
    float m_jumpForce = 8.0f;

};
