// src/games/JumpGame.cpp
#include "JumpGame.hpp"
#include "../core/ResourceManager.hpp"
#include <cstdlib>
#include <cmath>

JumpGame::JumpGame() {
    m_pouX = 427.0f;
    m_pouY = 350.0f;
    m_gameOver = false;
    m_wantsExit = false;
}

void JumpGame::start(PouState& state) {
    m_pouX = 427.0f;
    m_pouY = 350.0f;
    m_pouVy = -680.0f;
    m_highestY = 0.0f;
    m_score = 0;
    m_coinsEarned = 0;
    m_gameOver = false;
    m_wantsExit = false;
    m_platforms.clear();

    // Base starting platform
    JumpPlatform base;
    base.x = 427.0f;
    base.y = 420.0f;
    base.w = 120.0f;
    base.h = 24.0f;
    base.isMoving = false;
    base.vx = 0.0f;
    base.hasCoin = false;
    base.coinCollected = false;
    base.texPath = "images/games/jump/ground1.png";
    m_platforms.push_back(base);

    // Initial stack of platforms
    m_topmostPlatformY = 420.0f;
    for (int i = 0; i < 7; ++i) {
        m_topmostPlatformY -= 65.0f;
        spawnPlatform(m_topmostPlatformY);
    }

    ResourceManager::getInstance().playMusic("music_jump.wav", true);
}

void JumpGame::spawnPlatform(float y) {
    JumpPlatform p;
    p.x = 80.0f + (rand() % (PouConfig::DRC_WIDTH - 160));
    p.y = y;
    p.w = 95.0f;
    p.h = 22.0f;
    p.isMoving = ((rand() % 100) < 30);
    p.vx = p.isMoving ? (100.0f + (rand() % 80)) : 0.0f;
    p.hasCoin = ((rand() % 100) < 40);
    p.coinCollected = false;

    // Platform graphic based on height
    if (m_score > 60) {
        p.texPath = "images/games/jump/planet1.png";
    } else if (m_score > 30) {
        p.texPath = "images/games/jump/cloud.png";
    } else {
        p.texPath = "images/games/jump/ground2.png";
    }

    m_platforms.push_back(p);
}

void JumpGame::update(float dt, const InputState& input, PouState& state) {
    if (m_gameOver) {
        if (input.touchJustPressed || input.btnA || input.btnB) {
            m_wantsExit = true;
            ResourceManager::getInstance().stopMusic();
        }
        return;
    }

    // Steering
    float steerSpeed = 460.0f;
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

    // Screen wrapping
    if (m_pouX < -20.0f) m_pouX = PouConfig::DRC_WIDTH + 20.0f;
    if (m_pouX > PouConfig::DRC_WIDTH + 20.0f) m_pouX = -20.0f;

    // Gravity & physics
    m_pouVy += 1100.0f * dt;
    m_pouY += m_pouVy * dt;

    // Moving platforms
    for (auto& p : m_platforms) {
        if (p.isMoving) {
            p.x += p.vx * dt;
            if (p.x - p.w / 2.0f <= 30.0f || p.x + p.w / 2.0f >= PouConfig::DRC_WIDTH - 30.0f) {
                p.vx = -p.vx;
            }
        }
    }

    // Platform bounce landing (only when falling downwards)
    if (m_pouVy > 0.0f) {
        for (auto& p : m_platforms) {
            float platLeft = p.x - p.w / 2.0f;
            float platRight = p.x + p.w / 2.0f;

            if (m_pouX >= platLeft && m_pouX <= platRight &&
                m_pouY + 28.0f >= p.y - 12.0f && m_pouY + 28.0f <= p.y + 16.0f) {
                // Bounce!
                m_pouVy = -680.0f;
                ResourceManager::getInstance().playSound("ball.wav", 80);
                break;
            }
        }
    }

    // Coin collection
    for (auto& p : m_platforms) {
        if (p.hasCoin && !p.coinCollected) {
            float distCoin = std::sqrt(std::pow(m_pouX - p.x, 2) + std::pow(m_pouY - (p.y - 30.0f), 2));
            if (distCoin < 36.0f) {
                p.coinCollected = true;
                m_coinsEarned++;
                state.addCoins(1);
                ResourceManager::getInstance().playSound("coin.wav", 80);
            }
        }
    }

    // Camera scrolling when Pou goes above middle (y < 240)
    if (m_pouY < 240.0f) {
        float scrollDelta = 240.0f - m_pouY;
        m_pouY = 240.0f;
        m_score += static_cast<int>(scrollDelta * 0.1f);
        state.addXP(static_cast<int>(scrollDelta * 0.02f));

        for (auto& p : m_platforms) {
            p.y += scrollDelta;
        }
        m_topmostPlatformY += scrollDelta;

        // Spawn new platforms above
        while (m_topmostPlatformY > 20.0f) {
            m_topmostPlatformY -= (55.0f + (rand() % 25));
            spawnPlatform(m_topmostPlatformY);
        }
    }

    // Remove platforms that fell off bottom
    for (auto it = m_platforms.begin(); it != m_platforms.end();) {
        if (it->y > PouConfig::DRC_HEIGHT + 40.0f) {
            it = m_platforms.erase(it);
        } else {
            ++it;
        }
    }

    // Fallen off bottom = Game Over
    if (m_pouY > PouConfig::DRC_HEIGHT + 30.0f) {
        m_gameOver = true;
        ResourceManager::getInstance().playSound("game_over.wav", 110);
    }
}

void JumpGame::renderGameElements(SDL_Renderer* r, float scale, bool isTV) {
    // Sky gradient background
    SDL_SetRenderDrawColor(r, 195, 235, 255, 255);
    SDL_Rect bg = {0, 0, static_cast<int>(PouConfig::DRC_WIDTH * scale), static_cast<int>(PouConfig::DRC_HEIGHT * scale)};
    SDL_RenderFillRect(r, &bg);

    // Platforms
    for (const auto& p : m_platforms) {
        int pw = static_cast<int>(p.w * scale);
        int ph = static_cast<int>(p.h * scale);
        ResourceManager::getInstance().drawTextureCentered(r, p.texPath, static_cast<int>(p.x * scale), static_cast<int>(p.y * scale), pw, ph);

        if (p.hasCoin && !p.coinCollected) {
            int coinSize = static_cast<int>(28 * scale);
            ResourceManager::getInstance().drawTextureCentered(r, "images/coin/1.png", static_cast<int>(p.x * scale), static_cast<int>((p.y - 25.0f) * scale), coinSize, coinSize);
        }
    }

    // Pou
    float pouScale = 1.0f * scale;
    int px = static_cast<int>(m_pouX * scale);
    int py = static_cast<int>(m_pouY * scale);

    ResourceManager::fillCircle(r, px, py, static_cast<int>(32 * pouScale), {PouConfig::COLOR_POU_DEFAULT.r, PouConfig::COLOR_POU_DEFAULT.g, PouConfig::COLOR_POU_DEFAULT.b, 255});
    ResourceManager::drawCircle(r, px, py, static_cast<int>(32 * pouScale), {PouConfig::COLOR_POU_OUTLINE.r, PouConfig::COLOR_POU_OUTLINE.g, PouConfig::COLOR_POU_OUTLINE.b, 255}, 2);

    // Upward looking eyes
    ResourceManager::fillCircle(r, px - 10 * pouScale, py - 6 * pouScale, 8 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px + 10 * pouScale, py - 6 * pouScale, 8 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px - 10 * pouScale, py - 8 * pouScale, 4 * pouScale, {40, 40, 40, 255});
    ResourceManager::fillCircle(r, px + 10 * pouScale, py - 8 * pouScale, 4 * pouScale, {40, 40, 40, 255});

    // Happy smile
    ResourceManager::fillCircle(r, px, py + 8 * pouScale, 5 * pouScale, {50, 30, 20, 255});

    // HUD
    std::string scoreText = "Height: " + std::to_string(m_score);
    ResourceManager::getInstance().renderText(r, scoreText, static_cast<int>(40 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {40, 80, 110, 255});

    std::string coinsText = "Coins: " + std::to_string(m_coinsEarned);
    ResourceManager::getInstance().renderText(r, coinsText, static_cast<int>(340 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {220, 160, 20, 255});

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

void JumpGame::renderTV(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 2.25f, true);
}

void JumpGame::renderDRC(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 1.0f, false);
}
