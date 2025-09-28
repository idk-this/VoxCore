//
// Created by IDKTHIS on 23.09.2025.
//

#pragma once
#include "AContoller.h"
#include "UPlayer.h"
#include "Core/ECS/Player/APawn.h"

class APlayerController : public AController {
    UCLASS(APlayerController);
public:
    void SetPlayer(std::shared_ptr<UPlayer> player)
    {

        m_player = player;
        player->SetController(this);
    }
    std::shared_ptr<UPlayer> GetPlayer() const { return m_player; }

    virtual void Possess(std::shared_ptr<APawn> pawn) override {
        AController::Possess(pawn);
        if (pawn) pawn->SetController(this);
    }

    virtual void UnPossess() override {
        if (m_pawn) m_pawn->SetController(nullptr);
        AController::UnPossess();
    }

private:
    std::shared_ptr<UPlayer> m_player;
};
