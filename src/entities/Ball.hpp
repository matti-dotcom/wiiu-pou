// src/entities/Ball.hpp
#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "../core/PouState.hpp"

class Ball {
public:
    Ball();

    void init(float startX, float startY, float radius);
    void update(float dt, PouState& state, float minX, float maxX, float groundY);
    void render(SDL_Renderer* renderer, const std::string& ballTextureName, float scale = 1.0f);

    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getRadius() const { return m_radius; }

private:
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_vx = 0.0f;
    float m_vy = 0.0f;
    float m_radius = 35.0f;
    float m_rotation = 0.0f;

    bool m_isDragged = false;
    float m_lastTouchX = 0.0f;
    float m_lastTouchY = 0.0f;
    float m_dragVx = 0.0f;
    float m_dragVy = 0.0f;
};
