//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include <memory>
#include <vector>

#include "Core/ECS/Base/UObject.h"
#include "Core/ECS/Systems/UBaseSystem.h"

class AActor; // forward

class UBaseComponent : public UObject {
    UCLASS(UBaseComponent);
public:
    UBaseComponent() = default;
    virtual ~UBaseComponent() = default;

    virtual void OnAttach(AActor* owner) { m_owner = owner; }
    virtual void OnDetach() {}

    virtual void Update(float deltaTime) {
        for (auto& sys : m_systems) {
            sys->Update(deltaTime);
        }
    }

    void AddSystem(std::shared_ptr<UBaseSystem> system) {
        system->AttachToComponent(this);
        m_systems.push_back(system);
    }

    template<typename T>
    T* GetSystem() {
        for (auto& sys : m_systems) {
            if (auto ptr = dynamic_cast<T*>(sys.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    AActor* GetOwner() const { return m_owner; }

protected:
    AActor* m_owner = nullptr;
    std::vector<std::shared_ptr<UBaseSystem>> m_systems;
};