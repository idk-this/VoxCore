//
// Created by IDKTHIS on 23.09.2025.
//

#pragma once

#include <memory>


class APlayerController;

class UPlayer : public UObject {
    UCLASS(UPlayer);
public:
    void SetController(std::shared_ptr<APlayerController> controller) {
        m_controller = controller;
    }

    std::shared_ptr<APlayerController> GetController() {
        return m_controller;
    }

private:
    std::shared_ptr<APlayerController> m_controller;
};
