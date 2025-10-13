//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include "Core/ECS/Base/UObject.h"
#include <vector>
#include <typeindex>
#include "Core/Export.h"

class AActor;
class UBaseComponent;

class VOXCORE_API UBaseSystem : public UObject {
    UCLASS(UBaseSystem);
public:
    UBaseSystem() = default;
    virtual ~UBaseSystem() = default;

    virtual void Update(float deltaTime) = 0;

    virtual std::vector<std::type_index> GetRequiredComponents() const { return {}; }
    bool CheckActorComponents(AActor* actor);


    virtual void AttachToActor(AActor* actor) { m_ownerActor = actor; }
    virtual void AttachToComponent(UBaseComponent* component) { m_ownerComponent = component; }

protected:
    AActor* m_ownerActor = nullptr;
    UBaseComponent* m_ownerComponent = nullptr;
};

