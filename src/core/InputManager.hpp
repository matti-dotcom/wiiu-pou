// src/core/InputManager.hpp
#pragma once

#include <SDL2/SDL.h>
#include "Config.hpp"

#ifdef __WIIU__
#include <vpad/input.h>
#endif

struct InputState {
    // Touch on DRC (854x480 coordinates)
    int touchX = 0;
    int touchY = 0;
    bool isTouched = false;
    bool touchJustPressed = false;
    bool touchJustReleased = false;

    // Buttons
    bool homePressed = false;
    bool btnA = false;
    bool btnB = false;
    bool btnX = false;
    bool btnY = false;
    bool btnPlus = false;
    bool btnMinus = false;

    // D-Pad
    bool dpadLeft = false;
    bool dpadRight = false;
    bool dpadUp = false;
    bool dpadDown = false;

    // Left Analog Stick (-1.0 to 1.0)
    float stickX = 0.0f;
    float stickY = 0.0f;

    // Gyro / Accelerometer horizontal tilt angle (-1.0 to 1.0)
    float gyroTiltX = 0.0f;
};

class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance;
        return instance;
    }

    void init();
    void update(SDL_Window* drcWindow = nullptr);

    const InputState& getState() const { return m_state; }

private:
    InputManager() = default;
    InputState m_state;
    bool m_wasTouched = false;

#ifdef __WIIU__
    VPADStatus m_vpad;
    VPADReadError m_vpadError;
#endif
};
