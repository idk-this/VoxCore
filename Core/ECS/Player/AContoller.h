//
// Created by IDKTHIS on 23.09.2025.
//

#pragma once

#include <memory>
#include <utility>

#include "Core/ECS/Base/AActor.h"
#include "Core/ECS/Base/UObject.h"

class APawn;

class AController : public AActor {
    UCLASS(AController);
public:
    virtual void Possess(std::shared_ptr<APawn> pawn)
    {
        m_pawn = std::move(pawn);
    };
    virtual void UnPossess() {};

    std::shared_ptr<APawn> GetPawn() const { return m_pawn; }

protected:
    std::shared_ptr<APawn> m_pawn;
};