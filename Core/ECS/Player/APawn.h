//
// Created by IDKTHIS on 23.09.2025.
//

#pragma once
#include "AContoller.h"
#include "Core/ECS/Base/AActor.h"


class APawn : public AActor {
    UCLASS(APawn);
public:
    void SetController(AController* controller) { m_controller = controller; }
    AController* GetController() const { return m_controller; }

    void Update(float deltaTime) override {
        AActor::Update(deltaTime);
    }

private:
    AController* m_controller = nullptr;
};
