// src/entities/Ball.cpp
#include "Ball.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

Ball::Ball() {
    init(427.0f, 320.0f, 32.0f);
}

void Ball::init(float startX, float startY, float radius) {
    m_x = startX;
    m_y = startY;
    m_radius = radius;
    m_vx = 0.0f;
    m_vy = 0.0f;
    m_rotation = 0.0f;
    m_isDragged = false;
}

void Ball::update(float dt, PouState& state, float minX, float maxX, float groundY) {
    if (m_isDragged) {
        // Position updated by drag
        return;
    }

    // Apply gravity
    m_vy += 1200.0f * dt;
    m_x += m_vx * dt;
    m_y += m_vy * dt;

    // Air friction
    m_vx *= std::pow(0.985f, dt * 60.0f);

    // Rotation from horizontal motion
    m_rotation += (m_vx * dt * 180.0f) / (3.14159f * m_radius);

    // Floor collision
    if (m_y + m_radius >= groundY) {
        m_y = groundY - m_radius;
        if (std::abs(m_vy) > 40.0f) {
            m_vy = -m_vy * 0.72f;
            ResourceManager::getInstance().playSound("ball.wav", 64);
            state.playWithBall(1.0f);
        } else {
            m_vy = 0.0f;
        }
    }

    // Left wall collision
    if (m_x - m_radius <= minX) {
        m_x = minX + m_radius;
        m_vx = -m_vx * 0.75f;
        ResourceManager::getInstance().playSound("ball.wav", 40);
    }

    // Right wall collision
    if (m_x + m_radius >= maxX) {
        m_x = maxX - m_radius;
        m_vx = -m_vx * 0.75f;
        ResourceManager::getInstance().playSound("ball.wav", 40);
    }
}

void Ball::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state) {
    float dx = touchX - m_x;
    float dy = touchY - m_y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (justPressed && dist <= m_radius * 1.5f) {
        m_isDragged = true;
        m_lastTouchX = static_cast<float>(touchX);
        m_lastTouchY = static_cast<float>(touchY);
        m_vx = 0.0f;
        m_vy = 0.0f;
    } else if (m_isDragged && isTouched) {
        float curTouchX = static_cast<float>(touchX);
        float curTouchY = static_cast<float>(touchY);
        m_dragVx = (curTouchX - m_lastTouchX) * 45.0f;
        m_dragVy = (curTouchY - m_lastTouchY) * 45.0f;

        m_x = curTouchX;
        m_y = curTouchY;
        m_lastTouchX = curTouchX;
        m_lastTouchY = curTouchY;
    } else if (m_isDragged && justReleased) {
        m_isDragged = false;
        m_vx = m_dragVx;
        m_vy = m_dragVy;
        ResourceManager::getInstance().playSound("ball.wav", 90);
        state.playWithBall(3.0f);
    }
}

void Ball::render(SDL_Renderer* renderer, const std::string& ballTextureName, float scale) {
    std::string texPath = "images/balls/" + (ballTextureName.empty() ? "Basketball.png" : ballTextureName);
    int size = static_cast<int>(m_radius * 2.0f * scale);
    ResourceManager::getInstance().drawTextureCentered(renderer, texPath, static_cast<int>(m_x * scale), static_cast<int>(m_y * scale), size, size, m_rotation);
}
