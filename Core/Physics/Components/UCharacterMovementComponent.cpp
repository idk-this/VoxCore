//
// Created by IDKTHIS on 13.10.2025.
//

#include "UCharacterMovementComponent.h"

#include "Application/Application.h"
#include "Core/ECS/Base/UWorld.h"

void UCharacterMovementComponent::Update(float deltaTime)
{
    UBaseComponent::Update(deltaTime);

}

bool UCharacterMovementComponent::CheckAABBCollision(const FAABB& a, const FAABB& b, glm::vec3& pushOut)
{

    return false;
}
