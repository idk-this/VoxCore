//
// Created by IDKTHIS on 23.09.2025.
//

#pragma once


class APlayerController;

class UPlayer : public UObject {
    UCLASS(UPlayer);
public:
    void SetController(APlayerController* controller) {
        m_controller = controller;
    }

    APlayerController* GetController() {
        return m_controller;
    }

private:
    APlayerController* m_controller = nullptr;
};
