//
// Created by IDKTHIS on 16.07.2025.
//

#pragma once

#include <vector>
#include <memory>
#include "Core/Log/Logger.h"
#include "AActor.h"
#include "Core/ECS/Systems/UBaseSystem.h"
#include "Structures/FHitResult.h"

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
            if (!actor) continue;
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

    FHitResult LineTrace(const glm::vec3& start,
                     const glm::vec3& direction,
                     float maxDistance,
                     AActor* ignore = nullptr)
    {
        FHitResult result;
        result.Distance = maxDistance;

        glm::vec3 dir = glm::normalize(direction);

        for (auto& actor : m_actors) {
            if (!actor || actor.get() == ignore) continue;

            auto box = actor->GetBoundingBox();
            float tmin, tmax;
            if (RayIntersectsAABB(start, dir, box.Min, box.Max, tmin, tmax)) {
                float travelT = (tmin >= 0.0f) ? tmin : tmax;
                if (travelT >= 0.0f && travelT <= maxDistance && travelT < result.Distance) {
                    result.bHit = true;
                    result.HitActor = actor;
                    result.HitLocation = start + dir * travelT;
                    result.Distance = travelT;
                }

            }
        }

        return result;
    }



private:
    bool RayIntersectsAABB(const glm::vec3& rayOrigin,
                       const glm::vec3& rayDir,
                       const glm::vec3& boxMin,
                       const glm::vec3& boxMax,
                       float& tmin, float& tmax) const
    {
        if (rayOrigin.x >= boxMin.x && rayOrigin.x <= boxMax.x &&
            rayOrigin.y >= boxMin.y && rayOrigin.y <= boxMax.y &&
            rayOrigin.z >= boxMin.z && rayOrigin.z <= boxMax.z)
        {
            tmin = 0.0f;
            tmax = 0.0f;
            return true;
        }

        tmin = -FLT_MAX;
        tmax =  FLT_MAX;

        for (int i = 0; i < 3; i++) {
            if (fabs(rayDir[i]) < 1e-8f) {
                if (rayOrigin[i] < boxMin[i] || rayOrigin[i] > boxMax[i])
                    return false;
            } else {
                float invD = 1.0f / rayDir[i];
                float t0 = (boxMin[i] - rayOrigin[i]) * invD;
                float t1 = (boxMax[i] - rayOrigin[i]) * invD;
                if (t0 > t1) std::swap(t0, t1);
                tmin = std::max(tmin, t0);
                tmax = std::min(tmax, t1);
                if (tmax < tmin)
                    return false;
            }
        }

        return true;
    }


private:
    std::vector<uint32_t> m_generations;
    std::vector<uint32_t> m_freeList;
    std::vector<std::shared_ptr<AActor>> m_actors;
    std::vector<std::shared_ptr<UBaseSystem>> m_systems;
};
