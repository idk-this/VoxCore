//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once
#include <vector>
#include <memory>

#include "UObject.h"
#include "Core/ECS/Components/UBaseComponent.h"
#include "Core/ECS/Systems/UBaseSystem.h"
#include "Core/ECS/Components/UTransformComponent.h"
#include "Structures/FObjectID.h"

struct FAABB {
    glm::vec3 Min;
    glm::vec3 Max;
};

class AActor : public UObject {
    UCLASS(AActor);
public:
    AActor()
    {
        auto transform = std::make_shared<UTransformComponent>();
        transform->OnAttach(this);
        m_components.push_back(transform);
    };
    virtual ~AActor() = default;

    void AddComponent(std::shared_ptr<UBaseComponent> component) {
        component->OnAttach(this);
        m_components.push_back(component);
    }

    template<typename T>
    T* GetComponent() {
        for (auto& comp : m_components) {
            if (auto ptr = dynamic_cast<T*>(comp.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    void AddSystem(std::shared_ptr<UBaseSystem> system) {
        system->AttachToActor(this);
        m_systems.push_back(system);
    }
    virtual FAABB GetBoundingBox() {
        auto transform = GetComponent<UTransformComponent>();
        glm::vec3 pos = transform ? transform->position : glm::vec3(0.0f);
        float half = 0.5f;
        return { pos - glm::vec3(half), pos + glm::vec3(half) };
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
    void SetObjectID(const FObjectID& id) { m_objectID = id; }
    FObjectID GetObjectID() const { return m_objectID; }
    virtual void Update(float deltaTime) {
        for (auto& system : m_systems) {
            system->Update(deltaTime);
        }
        for (auto& comp : m_components) {
            comp->Update(deltaTime);
        }
    }

protected:
    FObjectID m_objectID;
    std::vector<std::shared_ptr<UBaseComponent>> m_components;
    std::vector<std::shared_ptr<UBaseSystem>> m_systems;
};
