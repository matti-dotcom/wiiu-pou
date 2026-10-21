// src/core/InputManager.cpp
#include "InputManager.hpp"
#include <algorithm>
#include <cmath>

void InputManager::init() {
    m_state = InputState();
    m_wasTouched = false;
}

void InputManager::update(SDL_Window* drcWindow) {
    bool previouslyTouched = m_wasTouched;
    m_state.touchJustPressed = false;
    m_state.touchJustReleased = false;
    m_state.homePressed = false;

#ifdef __WIIU__
    // Read hardware VPAD from Wii U GamePad
    VPADRead(VPAD_CHAN_0, &m_vpad, 1, &m_vpadError);

    if (m_vpadError == VPAD_READ_SUCCESS) {
        if (m_vpad.trigger & VPAD_BUTTON_HOME) {
            m_state.homePressed = true;
        }

        m_state.btnA = (m_vpad.hold & VPAD_BUTTON_A);
        m_state.btnB = (m_vpad.hold & VPAD_BUTTON_B);
        m_state.btnX = (m_vpad.hold & VPAD_BUTTON_X);
        m_state.btnY = (m_vpad.hold & VPAD_BUTTON_Y);
        m_state.btnPlus = (m_vpad.hold & VPAD_BUTTON_PLUS);
        m_state.btnMinus = (m_vpad.hold & VPAD_BUTTON_MINUS);

        m_state.dpadLeft = (m_vpad.hold & VPAD_BUTTON_LEFT);
        m_state.dpadRight = (m_vpad.hold & VPAD_BUTTON_RIGHT);
        m_state.dpadUp = (m_vpad.hold & VPAD_BUTTON_UP);
        m_state.dpadDown = (m_vpad.hold & VPAD_BUTTON_DOWN);

        // Analog stick normalized (-1.0f to 1.0f)
        m_state.stickX = m_vpad.leftStick.x;
        m_state.stickY = m_vpad.leftStick.y;

        // Gyroscope tilt angle
        m_state.gyroTiltX = std::clamp(m_vpad.angle.x, -1.0f, 1.0f);

        // Touch input
        if (m_vpad.tp.touched) {
            VPADTouchData touch;
            VPADGetCrossHairPoint(VPAD_CHAN_0, &touch);

            // Calibrated touch mapping to DRC 854x480 resolution
            m_state.touchX = std::clamp(static_cast<int>((m_vpad.tp.x / 1280.0f) * PouConfig::DRC_WIDTH), 0, PouConfig::DRC_WIDTH);
            m_state.touchY = std::clamp(static_cast<int>((m_vpad.tp.y / 720.0f) * PouConfig::DRC_HEIGHT), 0, PouConfig::DRC_HEIGHT);
            m_state.isTouched = true;
        } else {
            m_state.isTouched = false;
        }
    }
#else
    // Desktop simulation via SDL events / mouse / keyboard
    int mouseX, mouseY;
    uint32_t mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

    if (drcWindow) {
        int winW, winH;
        SDL_GetWindowSize(drcWindow, &winW, &winH);
        if (winW > 0 && winH > 0) {
            m_state.touchX = std::clamp((mouseX * PouConfig::DRC_WIDTH) / winW, 0, PouConfig::DRC_WIDTH);
            m_state.touchY = std::clamp((mouseY * PouConfig::DRC_HEIGHT) / winH, 0, PouConfig::DRC_HEIGHT);
        }
    } else {
        m_state.touchX = mouseX;
        m_state.touchY = mouseY;
    }

    m_state.isTouched = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

    const uint8_t* keystate = SDL_GetKeyboardState(nullptr);
    m_state.dpadLeft = keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A];
    m_state.dpadRight = keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D];
    m_state.dpadUp = keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W];
    m_state.dpadDown = keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_S];

    m_state.btnA = keystate[SDL_SCANCODE_SPACE] || keystate[SDL_SCANCODE_RETURN];
    m_state.btnB = keystate[SDL_SCANCODE_ESCAPE] || keystate[SDL_SCANCODE_BACKSPACE];

    m_state.stickX = 0.0f;
    if (m_state.dpadLeft) m_state.stickX -= 1.0f;
    if (m_state.dpadRight) m_state.stickX += 1.0f;
    m_state.gyroTiltX = m_state.stickX * 0.5f;
#endif

    // Transitions
    if (m_state.isTouched && !previouslyTouched) {
        m_state.touchJustPressed = true;
    } else if (!m_state.isTouched && previouslyTouched) {
        m_state.touchJustReleased = true;
    }

    m_wasTouched = m_state.isTouched;
}
