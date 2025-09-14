//
// Created by IDKTHIS on 28.07.2025.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "Keys.h"
#pragma once
#include <unordered_map>
#include <cstdint>



class WindowInputComponent {
public:
    void BeginFrame();
    KeyState GetKeyState(KeyCode key) const;
    bool IsKeyDown(KeyCode key) const;
    bool IsKeyPressed(KeyCode key) const;
    bool IsKeyReleased(KeyCode key) const;
    const MouseState& GetMouseState() const { return m_mouse; }

    void UpdateKeyState(KeyCode key, bool pressed);
    void UpdateMouseMotion(int x, int y, int dx, int dy);
    void SetMouseButtonState(int button, bool pressed);
private:
    std::unordered_map<KeyCode, KeyState> m_keyStates;
    MouseState m_mouse;
};

