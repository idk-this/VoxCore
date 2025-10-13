//
// Created by IDKTHIS on 13.10.2025.
//
#include "UBaseSystem.h"

#include "Core/ECS/Base/AActor.h"

bool UBaseSystem::CheckActorComponents(AActor* actor)
{
    if (!actor) return false;

    const auto requiredComponents = GetRequiredComponents();
    if (requiredComponents.empty()) return true;

    for (const auto& type : requiredComponents)
    {
        if (!actor->HasComponent(type))
            return false;
    }

    return true;
}
