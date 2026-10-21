// src/games/FallGame.cpp
#include "FallGame.hpp"
#include "../core/ResourceManager.hpp"
#include <cstdlib>
#include <cmath>

FallGame::FallGame() {
    m_pouX = 427.0f;
    m_pouY = 130.0f;
    m_gameOver = false;
    m_wantsExit = false;
}

void FallGame::start(PouState& state) {
    m_pouX = 427.0f;
    m_pouY = 130.0f;
    m_clouds.clear();
    m_score = 0;
    m_coinsEarned = 0;
    m_hits = 0;
    m_fallSpeed = 260.0f;
    m_gameOver = false;
    m_wantsExit = false;
    m_spawnTimer = 0.0f;
    m_spawnInterval = 1.2f;

    ResourceManager::getInstance().playMusic("music_fall.wav", true);
}

void FallGame::spawnCloudPair() {
    // Gap size for Pou to navigate
    float gapSize = 180.0f;
    float gapCenter = 150.0f + (rand() % (PouConfig::DRC_WIDTH - 300));

    // Cloud on left
    FallCloud left;
    left.x = gapCenter - gapSize / 2.0f - 200.0f;
    left.y = PouConfig::DRC_HEIGHT + 30.0f;
    left.w = 400.0f;
    left.h = 75.0f;
    left.hasCoin = false;
    left.coinCollected = false;
    m_clouds.push_back(left);

    // Cloud on right
    FallCloud right;
    right.x = gapCenter + gapSize / 2.0f + 200.0f;
    right.y = PouConfig::DRC_HEIGHT + 30.0f;
    right.w = 400.0f;
    right.h = 75.0f;
    right.hasCoin = ((rand() % 100) < 50);
    right.coinCollected = false;
    right.coinX = gapCenter;
    right.coinY = right.y;
    m_clouds.push_back(right);
}

void FallGame::update(float dt, const InputState& input, PouState& state) {
    if (m_gameOver) {
        if (input.touchJustPressed || input.btnA || input.btnB) {
            m_wantsExit = true;
            ResourceManager::getInstance().stopMusic();
        }
        return;
    }

    // Steering: Gyroscope tilt, stick, d-pad, touch
    float steerSpeed = 420.0f;
    if (std::abs(input.gyroTiltX) > 0.05f) {
        m_pouX += input.gyroTiltX * steerSpeed * dt * 1.5f;
    }
    if (input.dpadLeft || input.stickX < -0.2f) {
        m_pouX -= steerSpeed * dt;
    }
    if (input.dpadRight || input.stickX > 0.2f) {
        m_pouX += steerSpeed * dt;
    }
    if (input.isTouched) {
        float touchDiff = input.touchX - m_pouX;
        m_pouX += std::clamp(touchDiff, -steerSpeed * dt, steerSpeed * dt);
    }
    m_pouX = std::clamp(m_pouX, 50.0f, PouConfig::DRC_WIDTH - 50.0f);

    // Spawn clouds
    m_spawnTimer += dt;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0f;
        m_spawnInterval = std::max(0.65f, 1.2f - (m_score * 0.015f));
        spawnCloudPair();
    }

    // Move clouds upwards
    m_fallSpeed = 260.0f + (m_score * 5.0f);
    for (auto it = m_clouds.begin(); it != m_clouds.end();) {
        it->y -= m_fallSpeed * dt;
        it->coinY -= m_fallSpeed * dt;

        // Collision with Pou
        float cloudLeft = it->x - it->w / 2.0f;
        float cloudRight = it->x + it->w / 2.0f;
        float cloudTop = it->y - it->h / 2.0f;
        float cloudBottom = it->y + it->h / 2.0f;

        if (m_pouX >= cloudLeft && m_pouX <= cloudRight &&
            m_pouY >= cloudTop && m_pouY <= cloudBottom) {
            // Hit cloud!
            m_hits++;
            ResourceManager::getInstance().playSound("cloud.wav", 100);
            it->y -= 100.0f; // Push cloud past Pou
            if (m_hits >= MAX_HITS) {
                m_gameOver = true;
                ResourceManager::getInstance().playSound("game_over.wav", 110);
            }
        }

        // Coin collection
        if (it->hasCoin && !it->coinCollected) {
            float distCoin = std::sqrt(std::pow(m_pouX - it->coinX, 2) + std::pow(m_pouY - it->coinY, 2));
            if (distCoin < 40.0f) {
                it->coinCollected = true;
                m_coinsEarned++;
                state.addCoins(1);
                ResourceManager::getInstance().playSound("coin.wav", 80);
            }
        }

        // Check if cloud passed top
        if (it->y < -100.0f) {
            if (it->w > 300.0f && it->hasCoin) { // Count one per pair
                m_score++;
                state.addXP(2);
            }
            it = m_clouds.erase(it);
        } else {
            ++it;
        }
    }
}

void FallGame::renderGameElements(SDL_Renderer* r, float scale, bool isTV) {
    // Sky blue background
    SDL_SetRenderDrawColor(r, 160, 215, 255, 255);
    SDL_Rect bg = {0, 0, static_cast<int>(PouConfig::DRC_WIDTH * scale), static_cast<int>(PouConfig::DRC_HEIGHT * scale)};
    SDL_RenderFillRect(r, &bg);

    // Clouds
    for (const auto& c : m_clouds) {
        int cw = static_cast<int>(c.w * scale);
        int ch = static_cast<int>(c.h * scale);
        ResourceManager::getInstance().drawTextureCentered(r, "images/games/fall/cloud.png", static_cast<int>(c.x * scale), static_cast<int>(c.y * scale), cw, ch);

        if (c.hasCoin && !c.coinCollected) {
            int coinSize = static_cast<int>(32 * scale);
            ResourceManager::getInstance().drawTextureCentered(r, "images/coin/1.png", static_cast<int>(c.coinX * scale), static_cast<int>(c.coinY * scale), coinSize, coinSize);
        }
    }

    // Pou diving
    float pouScale = 1.0f * scale;
    int px = static_cast<int>(m_pouX * scale);
    int py = static_cast<int>(m_pouY * scale);

    ResourceManager::fillCircle(r, px, py, static_cast<int>(35 * pouScale), {PouConfig::COLOR_POU_DEFAULT.r, PouConfig::COLOR_POU_DEFAULT.g, PouConfig::COLOR_POU_DEFAULT.b, 255});
    ResourceManager::drawCircle(r, px, py, static_cast<int>(35 * pouScale), {PouConfig::COLOR_POU_OUTLINE.r, PouConfig::COLOR_POU_OUTLINE.g, PouConfig::COLOR_POU_OUTLINE.b, 255}, 2);

    // Downward diving eyes
    ResourceManager::fillCircle(r, px - 11 * pouScale, py - 4 * pouScale, 8 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px + 11 * pouScale, py - 4 * pouScale, 8 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px - 11 * pouScale, py + 1 * pouScale, 4 * pouScale, {40, 40, 40, 255});
    ResourceManager::fillCircle(r, px + 11 * pouScale, py + 1 * pouScale, 4 * pouScale, {40, 40, 40, 255});

    // Mouth
    ResourceManager::fillCircle(r, px, py + 12 * pouScale, 6 * pouScale, {50, 30, 20, 255});

    // HUD
    std::string scoreText = "Clouds Dodged: " + std::to_string(m_score);
    ResourceManager::getInstance().renderText(r, scoreText, static_cast<int>(40 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {40, 70, 110, 255});

    std::string coinsText = "Coins: " + std::to_string(m_coinsEarned);
    ResourceManager::getInstance().renderText(r, coinsText, static_cast<int>(340 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {220, 160, 20, 255});

    std::string hitsText = "Hits: " + std::to_string(m_hits) + " / " + std::to_string(MAX_HITS);
    ResourceManager::getInstance().renderText(r, hitsText, static_cast<int>(700 * scale), static_cast<int>(25 * scale), static_cast<int>(22 * scale), {200, 50, 50, 255}, true);

    // Game Over
    if (m_gameOver) {
        int cx = static_cast<int>(PouConfig::DRC_WIDTH * 0.5f * scale);
        int cy = static_cast<int>(PouConfig::DRC_HEIGHT * 0.5f * scale);

        SDL_Rect card = {cx - static_cast<int>(180 * scale), cy - static_cast<int>(110 * scale), static_cast<int>(360 * scale), static_cast<int>(220 * scale)};
        ResourceManager::fillRoundedRect(r, card, 16, {252, 246, 235, 245});
        SDL_SetRenderDrawColor(r, 200, 160, 120, 255);
        SDL_RenderDrawRect(r, &card);

        ResourceManager::getInstance().renderText(r, "GAME OVER", cx, cy - static_cast<int>(65 * scale), static_cast<int>(36 * scale), {180, 50, 40, 255}, true);
        std::string finalScore = "Final Score: " + std::to_string(m_score);
        ResourceManager::getInstance().renderText(r, finalScore, cx, cy - static_cast<int>(15 * scale), static_cast<int>(26 * scale), {80, 50, 30, 255}, true);
        ResourceManager::getInstance().renderText(r, "Tap screen or press A to return", cx, cy + static_cast<int>(45 * scale), static_cast<int>(18 * scale), {120, 90, 70, 255}, true);
    }
}

void FallGame::renderTV(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 2.25f, true);
}

void FallGame::renderDRC(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 1.0f, false);
}
