//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include "Core/ECS/Base/UObject.h"

class AActor;
class UBaseComponent;

class UBaseSystem : public UObject {
    GENERATED_BODY();
public:
    UBaseSystem() = default;
    virtual ~UBaseSystem() = default;

    virtual void Update(float deltaTime) = 0;

    // Возможность прикрепления к актёру или компоненту
    virtual void AttachToActor(AActor* actor) { m_ownerActor = actor; }
    virtual void AttachToComponent(UBaseComponent* component) { m_ownerComponent = component; }

protected:
    AActor* m_ownerActor = nullptr;
    UBaseComponent* m_ownerComponent = nullptr;
};