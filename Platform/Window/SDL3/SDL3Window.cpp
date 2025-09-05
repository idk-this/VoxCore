//
// Created by IDKTHIS on 02.07.2025.
//

#include "SDL3Window.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include "../../../Core/Log/Logger.h"
#include "Platform/Window/Components/WindowInputComponent.h"

KeyCode ConvertKey(SDL_Scancode scancode);

SDL3Window::SDL3Window() = default;

SDL3Window::~SDL3Window() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

bool SDL3Window::Create(int width, int height, const std::string &title) {
    IWindow::Create(width, height, title);
    int SDLInitStatus = SDL_Init(SDL_INIT_VIDEO);
    if (SDLInitStatus != 1)
    {
        LOG_FATAL("Window", "Failed to initialize SDL - {}", SDL_GetError());
        return false;
    }
    m_window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN);
    inputComponent = new WindowInputComponent();

    return m_window != nullptr;
}

void SDL3Window::PollEvents() {
    inputComponent->BeginFrame();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                m_shouldClose = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                inputComponent->UpdateKeyState(ConvertKey(event.key.scancode), true);
                break;
            case SDL_EVENT_KEY_UP:
                inputComponent->UpdateKeyState(ConvertKey(event.key.scancode), false);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                inputComponent->UpdateMouseMotion(event.motion.x, event.motion.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                inputComponent->SetMouseButtonState(event.button.button, true);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                inputComponent->SetMouseButtonState(event.button.button, false);
                break;
            default:
                break;
        }
    }
}

void SDL3Window::SwapBuffers() {
    // No-op for Vulkan, handled by renderer
}

bool SDL3Window::ShouldClose() const {
    return m_shouldClose;
}

void *SDL3Window::GetNativeHandle() {
    return static_cast<void*>(m_window);
}

void SDL3Window::SetRelativeMouseMode(bool enable) {
    SDL_SetWindowMouseGrab(m_window, enable);
    enable ? SDL_HideCursor() : SDL_ShowCursor();
    SDL_SetWindowRelativeMouseMode(m_window, enable);
}


void SDL3Window::SetTitle(const std::string& title) {
    if (m_window) {
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

inline KeyCode ConvertKey(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_A: return KeyCode::KEY_A;
        case SDL_SCANCODE_B: return KeyCode::KEY_B;
        case SDL_SCANCODE_C: return KeyCode::KEY_C;
        case SDL_SCANCODE_D: return KeyCode::KEY_D;
        case SDL_SCANCODE_E: return KeyCode::KEY_E;
        case SDL_SCANCODE_F: return KeyCode::KEY_F;
        case SDL_SCANCODE_G: return KeyCode::KEY_G;
        case SDL_SCANCODE_H: return KeyCode::KEY_H;
        case SDL_SCANCODE_I: return KeyCode::KEY_I;
        case SDL_SCANCODE_J: return KeyCode::KEY_J;
        case SDL_SCANCODE_K: return KeyCode::KEY_K;
        case SDL_SCANCODE_L: return KeyCode::KEY_L;
        case SDL_SCANCODE_M: return KeyCode::KEY_M;
        case SDL_SCANCODE_N: return KeyCode::KEY_N;
        case SDL_SCANCODE_O: return KeyCode::KEY_O;
        case SDL_SCANCODE_P: return KeyCode::KEY_P;
        case SDL_SCANCODE_Q: return KeyCode::KEY_Q;
        case SDL_SCANCODE_R: return KeyCode::KEY_R;
        case SDL_SCANCODE_S: return KeyCode::KEY_S;
        case SDL_SCANCODE_T: return KeyCode::KEY_T;
        case SDL_SCANCODE_U: return KeyCode::KEY_U;
        case SDL_SCANCODE_V: return KeyCode::KEY_V;
        case SDL_SCANCODE_W: return KeyCode::KEY_W;
        case SDL_SCANCODE_X: return KeyCode::KEY_X;
        case SDL_SCANCODE_Y: return KeyCode::KEY_Y;
        case SDL_SCANCODE_Z: return KeyCode::KEY_Z;

        case SDL_SCANCODE_1: return KeyCode::KEY_1;
        case SDL_SCANCODE_2: return KeyCode::KEY_2;
        case SDL_SCANCODE_3: return KeyCode::KEY_3;
        case SDL_SCANCODE_4: return KeyCode::KEY_4;
        case SDL_SCANCODE_5: return KeyCode::KEY_5;
        case SDL_SCANCODE_6: return KeyCode::KEY_6;
        case SDL_SCANCODE_7: return KeyCode::KEY_7;
        case SDL_SCANCODE_8: return KeyCode::KEY_8;
        case SDL_SCANCODE_9: return KeyCode::KEY_9;
        case SDL_SCANCODE_0: return KeyCode::KEY_0;

        case SDL_SCANCODE_F1: return KeyCode::KEY_F1;
        case SDL_SCANCODE_F2: return KeyCode::KEY_F2;
        case SDL_SCANCODE_F3: return KeyCode::KEY_F3;
        case SDL_SCANCODE_F4: return KeyCode::KEY_F4;
        case SDL_SCANCODE_F5: return KeyCode::KEY_F5;
        case SDL_SCANCODE_F6: return KeyCode::KEY_F6;
        case SDL_SCANCODE_F7: return KeyCode::KEY_F7;
        case SDL_SCANCODE_F8: return KeyCode::KEY_F8;
        case SDL_SCANCODE_F9: return KeyCode::KEY_F9;
        case SDL_SCANCODE_F10: return KeyCode::KEY_F10;
        case SDL_SCANCODE_F11: return KeyCode::KEY_F11;
        case SDL_SCANCODE_F12: return KeyCode::KEY_F12;
        case SDL_SCANCODE_F13: return KeyCode::KEY_F13;
        case SDL_SCANCODE_F14: return KeyCode::KEY_F14;
        case SDL_SCANCODE_F15: return KeyCode::KEY_F15;
        case SDL_SCANCODE_F16: return KeyCode::KEY_F16;
        case SDL_SCANCODE_F17: return KeyCode::KEY_F17;
        case SDL_SCANCODE_F18: return KeyCode::KEY_F18;
        case SDL_SCANCODE_F19: return KeyCode::KEY_F19;
        case SDL_SCANCODE_F20: return KeyCode::KEY_F20;
        case SDL_SCANCODE_F21: return KeyCode::KEY_F21;
        case SDL_SCANCODE_F22: return KeyCode::KEY_F22;
        case SDL_SCANCODE_F23: return KeyCode::KEY_F23;
        case SDL_SCANCODE_F24: return KeyCode::KEY_F24;

        case SDL_SCANCODE_ESCAPE: return KeyCode::KEY_ESCAPE;
        case SDL_SCANCODE_TAB: return KeyCode::KEY_TAB;
        case SDL_SCANCODE_CAPSLOCK: return KeyCode::KEY_CAPS_LOCK;
        case SDL_SCANCODE_LSHIFT: return KeyCode::KEY_LEFT_SHIFT;
        case SDL_SCANCODE_RSHIFT: return KeyCode::KEY_RIGHT_SHIFT;
        case SDL_SCANCODE_LCTRL: return KeyCode::KEY_LEFT_CONTROL;
        case SDL_SCANCODE_RCTRL: return KeyCode::KEY_RIGHT_CONTROL;
        case SDL_SCANCODE_LALT: return KeyCode::KEY_LEFT_ALT;
        case SDL_SCANCODE_RALT: return KeyCode::KEY_RIGHT_ALT;
        case SDL_SCANCODE_LGUI: return KeyCode::KEY_LEFT_SUPER;
        case SDL_SCANCODE_RGUI: return KeyCode::KEY_RIGHT_SUPER;
        case SDL_SCANCODE_MENU: return KeyCode::KEY_MENU;

        case SDL_SCANCODE_GRAVE: return KeyCode::KEY_GRAVE;
        case SDL_SCANCODE_MINUS: return KeyCode::KEY_MINUS;
        case SDL_SCANCODE_EQUALS: return KeyCode::KEY_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return KeyCode::KEY_LEFT_BRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::KEY_RIGHT_BRACKET;
        case SDL_SCANCODE_BACKSLASH: return KeyCode::KEY_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON: return KeyCode::KEY_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return KeyCode::KEY_APOSTROPHE;
        case SDL_SCANCODE_COMMA: return KeyCode::KEY_COMMA;
        case SDL_SCANCODE_PERIOD: return KeyCode::KEY_PERIOD;
        case SDL_SCANCODE_SLASH: return KeyCode::KEY_SLASH;


        case SDL_SCANCODE_SPACE: return KeyCode::KEY_SPACE;
        case SDL_SCANCODE_RETURN: return KeyCode::KEY_ENTER;
        case SDL_SCANCODE_BACKSPACE: return KeyCode::KEY_BACKSPACE;
        case SDL_SCANCODE_INSERT: return KeyCode::KEY_INSERT;
        case SDL_SCANCODE_DELETE: return KeyCode::KEY_DELETE;
        case SDL_SCANCODE_HOME: return KeyCode::KEY_HOME;
        case SDL_SCANCODE_END: return KeyCode::KEY_END;
        case SDL_SCANCODE_PAGEUP: return KeyCode::KEY_PAGE_UP;
        case SDL_SCANCODE_PAGEDOWN: return KeyCode::KEY_PAGE_DOWN;
        case SDL_SCANCODE_PRINTSCREEN: return KeyCode::KEY_PRINT_SCREEN;
        case SDL_SCANCODE_SCROLLLOCK: return KeyCode::KEY_SCROLL_LOCK;
        case SDL_SCANCODE_PAUSE: return KeyCode::KEY_PAUSE;


        case SDL_SCANCODE_LEFT: return KeyCode::KEY_LEFT;
        case SDL_SCANCODE_UP: return KeyCode::KEY_UP;
        case SDL_SCANCODE_RIGHT: return KeyCode::KEY_RIGHT;
        case SDL_SCANCODE_DOWN: return KeyCode::KEY_DOWN;


        case SDL_SCANCODE_NUMLOCKCLEAR: return KeyCode::KEY_NUM_LOCK;
        case SDL_SCANCODE_KP_0: return KeyCode::KEY_NUMPAD_0;
        case SDL_SCANCODE_KP_1: return KeyCode::KEY_NUMPAD_1;
        case SDL_SCANCODE_KP_2: return KeyCode::KEY_NUMPAD_2;
        case SDL_SCANCODE_KP_3: return KeyCode::KEY_NUMPAD_3;
        case SDL_SCANCODE_KP_4: return KeyCode::KEY_NUMPAD_4;
        case SDL_SCANCODE_KP_5: return KeyCode::KEY_NUMPAD_5;
        case SDL_SCANCODE_KP_6: return KeyCode::KEY_NUMPAD_6;
        case SDL_SCANCODE_KP_7: return KeyCode::KEY_NUMPAD_7;
        case SDL_SCANCODE_KP_8: return KeyCode::KEY_NUMPAD_8;
        case SDL_SCANCODE_KP_9: return KeyCode::KEY_NUMPAD_9;
        case SDL_SCANCODE_KP_DECIMAL: return KeyCode::KEY_NUMPAD_DECIMAL;
        case SDL_SCANCODE_KP_DIVIDE: return KeyCode::KEY_NUMPAD_DIVIDE;
        case SDL_SCANCODE_KP_MULTIPLY: return KeyCode::KEY_NUMPAD_MULTIPLY;
        case SDL_SCANCODE_KP_MINUS: return KeyCode::KEY_NUMPAD_SUBTRACT;
        case SDL_SCANCODE_KP_PLUS: return KeyCode::KEY_NUMPAD_ADD;
        case SDL_SCANCODE_KP_ENTER: return KeyCode::KEY_NUMPAD_ENTER;


        case SDL_SCANCODE_VOLUMEUP: return KeyCode::KEY_VOLUME_UP;
        case SDL_SCANCODE_VOLUMEDOWN: return KeyCode::KEY_VOLUME_DOWN;
        case SDL_SCANCODE_MUTE: return KeyCode::KEY_MUTE;

        default: return KeyCode::KEY_UNKNOWN;
    }
}

