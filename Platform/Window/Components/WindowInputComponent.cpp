//
// Created by IDKTHIS on 28.07.2025.
//

#include "WindowInputComponent.h"

void WindowInputComponent::UpdateKeyState(KeyCode key, bool pressed) {
    auto it = m_keyStates.find(key);
    if (pressed) {
        if (it == m_keyStates.end() || it->second == KeyState::Up)
            m_keyStates[key] = KeyState::Pressed;
        else
            m_keyStates[key] = KeyState::Down;
    } else {
        if (it != m_keyStates.end() && (it->second == KeyState::Down || it->second == KeyState::Pressed))
            m_keyStates[key] = KeyState::Released;
        else
            m_keyStates[key] = KeyState::Up;
    }
}

void WindowInputComponent::UpdateMouseMotion(int x, int y, int dx, int dy) {
    m_mouse.deltaX = dx;
    m_mouse.deltaY = dy;
    m_mouse.x = x;
    m_mouse.y = y;
}

void WindowInputComponent::SetMouseButtonState(int button, bool pressed) {
    if (button < 0 || button >= 6) return;
    m_mouse.buttons[button] = pressed;
}

void WindowInputComponent::BeginFrame() {
    for (auto& [key, state] : m_keyStates) {
        if (state == KeyState::Pressed) state = KeyState::Down;
        else if (state == KeyState::Released) state = KeyState::Up;
    }
    m_mouse.deltaX = 0;
    m_mouse.deltaY = 0;
}

KeyState WindowInputComponent::GetKeyState(KeyCode key) const {
    auto it = m_keyStates.find(key);
    if (it != m_keyStates.end()) return it->second;
    return KeyState::Up;
}

bool WindowInputComponent::IsKeyDown(KeyCode key) const {
    auto state = GetKeyState(key);
    return state == KeyState::Down || state == KeyState::Pressed;
}

bool WindowInputComponent::IsKeyPressed(KeyCode key) const {
    return GetKeyState(key) == KeyState::Pressed;
}

bool WindowInputComponent::IsKeyReleased(KeyCode key) const {
    return GetKeyState(key) == KeyState::Released;
}
