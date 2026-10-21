// src/games/FoodGame.cpp
#include "FoodGame.hpp"
#include "../core/ResourceManager.hpp"
#include <cstdlib>
#include <cmath>

FoodGame::FoodGame() {
    m_pouX = 427.0f;
    m_pouY = 410.0f;
    m_gameOver = false;
    m_wantsExit = false;
}

void FoodGame::start(PouState& state) {
    m_pouX = 427.0f;
    m_pouY = 410.0f;
    m_items.clear();
    m_score = 0;
    m_coinsEarned = 0;
    m_misses = 0;
    m_gameOver = false;
    m_wantsExit = false;
    m_spawnTimer = 0.0f;
    m_spawnInterval = 1.1f;

    ResourceManager::getInstance().playMusic("music_food.wav", true);
}

void FoodGame::spawnItem() {
    FallingItem item;
    item.x = 80.0f + (rand() % (PouConfig::DRC_WIDTH - 160));
    item.y = -40.0f;
    item.vy = 180.0f + (rand() % 90) + (m_score * 4.0f);
    item.size = 50.0f;

    // 25% chance for bad junk item
    if ((rand() % 100) < 25) {
        item.isBad = true;
        const char* badItems[] = {"images/games/food/shoe.png", "images/games/food/cloth.png", "images/games/food/horseshoe.png"};
        item.iconPath = badItems[rand() % 3];
    } else {
        item.isBad = false;
        const char* goodFoods[] = {"images/food/Apple.png", "images/food/Burger.png", "images/food/Cherries.png", "images/food/Banana Donut.png", "images/food/Chocolate Cake.png"};
        item.iconPath = goodFoods[rand() % 5];
    }

    m_items.push_back(item);
}

void FoodGame::update(float dt, const InputState& input, PouState& state) {
    if (m_gameOver) {
        if (input.touchJustPressed || input.btnA || input.btnB) {
            m_wantsExit = true;
            ResourceManager::getInstance().stopMusic();
        }
        return;
    }

    // Input: stylus touch drag, analog stick, or D-pad
    if (input.isTouched) {
        m_pouX = static_cast<float>(input.touchX);
    } else {
        if (input.dpadLeft || input.stickX < -0.2f) {
            m_pouX -= m_pouSpeed * dt;
        }
        if (input.dpadRight || input.stickX > 0.2f) {
            m_pouX += m_pouSpeed * dt;
        }
    }
    m_pouX = std::clamp(m_pouX, 60.0f, PouConfig::DRC_WIDTH - 60.0f);

    // Spawn falling items
    m_spawnTimer += dt;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer = 0.0f;
        m_spawnInterval = std::max(0.45f, 1.1f - (m_score * 0.02f));
        spawnItem();
    }

    // Update items and check collision with Pou
    for (auto it = m_items.begin(); it != m_items.end();) {
        it->y += it->vy * dt;

        // Collision with Pou mouth
        float dx = std::abs(it->x - m_pouX);
        float dy = std::abs(it->y - m_pouY);

        if (dx < 48.0f && dy < 40.0f) {
            if (it->isBad) {
                // Ate trash!
                m_misses++;
                ResourceManager::getInstance().playSound("warning.wav", 100);
                if (m_misses >= MAX_MISSES) {
                    m_gameOver = true;
                    ResourceManager::getInstance().playSound("game_over.wav", 110);
                }
            } else {
                // Ate delicious food!
                m_score++;
                state.feed(3.0f);
                if (m_score % 3 == 0) {
                    m_coinsEarned += 2;
                    state.addCoins(2);
                    ResourceManager::getInstance().playSound("coin.wav", 75);
                } else {
                    ResourceManager::getInstance().playSound("a7a7e7e7e7e.wav", 80);
                }
            }
            it = m_items.erase(it);
        } else if (it->y > PouConfig::DRC_HEIGHT + 30.0f) {
            // Item fell off screen
            if (!it->isBad) {
                // Missed good food
                m_misses++;
                ResourceManager::getInstance().playSound("warning.wav", 70);
                if (m_misses >= MAX_MISSES) {
                    m_gameOver = true;
                    ResourceManager::getInstance().playSound("game_over.wav", 110);
                }
            }
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }
}

void FoodGame::renderGameElements(SDL_Renderer* r, float scale, bool isTV) {
    // Sky gradient background
    SDL_Rect bg = {0, 0, static_cast<int>(PouConfig::DRC_WIDTH * scale), static_cast<int>(PouConfig::DRC_HEIGHT * scale)};
    SDL_SetRenderDrawColor(r, 255, 235, 205, 255);
    SDL_RenderFillRect(r, &bg);

    // Falling items
    for (const auto& item : m_items) {
        int itemSize = static_cast<int>(item.size * scale);
        ResourceManager::getInstance().drawTextureCentered(r, item.iconPath, static_cast<int>(item.x * scale), static_cast<int>(item.y * scale), itemSize, itemSize);
    }

    // Pou at bottom
    float pouScale = 1.0f * scale;
    int px = static_cast<int>(m_pouX * scale);
    int py = static_cast<int>(m_pouY * scale);

    // Draw simple mini Pou
    ResourceManager::fillCircle(r, px, py, static_cast<int>(38 * pouScale), {PouConfig::COLOR_POU_DEFAULT.r, PouConfig::COLOR_POU_DEFAULT.g, PouConfig::COLOR_POU_DEFAULT.b, 255});
    ResourceManager::drawCircle(r, px, py, static_cast<int>(38 * pouScale), {PouConfig::COLOR_POU_OUTLINE.r, PouConfig::COLOR_POU_OUTLINE.g, PouConfig::COLOR_POU_OUTLINE.b, 255}, 2);

    // Eyes
    ResourceManager::fillCircle(r, px - 12 * pouScale, py - 6 * pouScale, 9 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px + 12 * pouScale, py - 6 * pouScale, 9 * pouScale, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, px - 12 * pouScale, py - 8 * pouScale, 4 * pouScale, {40, 40, 40, 255});
    ResourceManager::fillCircle(r, px + 12 * pouScale, py - 8 * pouScale, 4 * pouScale, {40, 40, 40, 255});

    // Open mouth to catch
    ResourceManager::fillCircle(r, px, py + 12 * pouScale, 8 * pouScale, {50, 30, 20, 255});

    // Top HUD
    std::string scoreText = "Score: " + std::to_string(m_score);
    ResourceManager::getInstance().renderText(r, scoreText, static_cast<int>(40 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {80, 50, 25, 255});

    std::string coinsText = "Coins: " + std::to_string(m_coinsEarned);
    ResourceManager::getInstance().renderText(r, coinsText, static_cast<int>(240 * scale), static_cast<int>(25 * scale), static_cast<int>(24 * scale), {200, 150, 20, 255});

    // Lives / Misses
    std::string livesText = "Misses: " + std::to_string(m_misses) + " / " + std::to_string(MAX_MISSES);
    ResourceManager::getInstance().renderText(r, livesText, static_cast<int>(680 * scale), static_cast<int>(25 * scale), static_cast<int>(22 * scale), {200, 60, 60, 255}, true);

    // Game Over Card
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

void FoodGame::renderTV(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 2.25f, true);
}

void FoodGame::renderDRC(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 1.0f, false);
}
