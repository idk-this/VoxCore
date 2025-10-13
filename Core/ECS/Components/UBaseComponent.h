//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include <memory>
#include <vector>
#include <typeinfo>

#include "Core/ECS/Base/UObject.h"
#include "Core/ECS/Systems/UBaseSystem.h"

class AActor;

class UBaseComponent : public UObject {
    UCLASS(UBaseComponent);
public:
    UBaseComponent() = default;
    virtual ~UBaseComponent() = default;

    virtual void OnAttach(AActor* owner) { m_owner = owner; }
    virtual void OnDetach() {}

    virtual void Update(float deltaTime) {
    }
    AActor* GetOwner() const { return m_owner; }

protected:
    AActor* m_owner = nullptr;
};