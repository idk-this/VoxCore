//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once

#include <vector>
#include <memory>

#include "AActor.h"
#include "Core/ECS/Systems/UBaseSystem.h"

class UWorld : public UObject {
    UCLASS(UWorld);
public:
    UWorld() = default;
    ~UWorld() = default;

    template<typename T, typename... Args>
     std::shared_ptr<T> SpawnActor(Args&&... args) {
        static_assert(std::is_base_of_v<AActor, T>, "T must inherit from AActor");

        auto actor = std::make_shared<T>(std::forward<Args>(args)...);
        FObjectID id;
        if (!m_freeList.empty()) {
            id.index = m_freeList.back();
            m_freeList.pop_back();
            id.generation = ++m_generations[id.index];
            m_actors[id.index] = actor;
        } else {
            id.index = static_cast<uint32_t>(m_actors.size());
            id.generation = 1;
            m_generations.push_back(id.generation);
            m_actors.push_back(actor);
        }

        actor->SetObjectID(id);
        return actor;
    }

    const std::vector<std::shared_ptr<AActor>>& GetActors() const {
        return m_actors;
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> AddSystem(Args&&... args) {
        auto system = std::make_shared<T>(std::forward<Args>(args)...);
        m_systems.push_back(system);
        return system;
    }

    void Update(float deltaTime) {
        for (auto& system : m_systems) {
            system->Update(deltaTime);
        }
        for (auto& actor : m_actors) {
            actor->Update(deltaTime);
        }
    }
    [[nodiscard]] std::shared_ptr<AActor> GetActor(FObjectID id) const {
        if (id.index >= m_actors.size()) return nullptr;
        auto actor = m_actors[id.index];
        if (!actor) return nullptr;
        if (m_generations[id.index] != id.generation) return nullptr;
        return actor;
    }

    void DestroyActor(const std::shared_ptr<AActor>& actor) {
        FObjectID id = actor->GetObjectID();
        if (id.index < m_actors.size() && m_actors[id.index] == actor) {
            m_actors[id.index] = nullptr;
            m_freeList.push_back(id.index);
        }
    }

private:
    std::vector<uint32_t> m_generations;
    std::vector<uint32_t> m_freeList;
    std::vector<std::shared_ptr<AActor>> m_actors;
    std::vector<std::shared_ptr<UBaseSystem>> m_systems;
};