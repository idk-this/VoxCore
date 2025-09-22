//
// Created by IDKTHIS on 02.07.2025.
//

#pragma once
#include "Platform/Window/IWindow.h"


class UWorld;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool Init(IWindow *window, UWorld* world) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Cleanup() = 0;
    virtual void RenderFrame() = 0;
};
