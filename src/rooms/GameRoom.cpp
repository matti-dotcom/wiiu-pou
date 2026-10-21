// src/rooms/GameRoom.cpp
#include "GameRoom.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

GameRoom::GameRoom() {
    m_showGameMenu = false;
    m_requestedGame = PouConfig::GameId::NONE;
}

void GameRoom::enter(PouState& state, Pou& pou) {
    m_ball.init(550.0f, 320.0f, 30.0f);
    m_showGameMenu = false;
    m_requestedGame = PouConfig::GameId::NONE;
}

void GameRoom::update(float dt, PouState& state, Pou& pou) {
    if (!m_showGameMenu) {
        m_ball.update(dt, state, 50.0f, 804.0f, 440.0f);

        // Pou tracks ball with eyes
        float dx = (m_ball.getX() - 427.0f) * 0.08f;
        float dy = (m_ball.getY() - 250.0f) * 0.08f;
        pou.setTargetLook(dx, dy);
    } else {
        pou.clearTargetLook();
    }
}

void GameRoom::drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp) {
    std::string wpPath = "images/wallpapers/" + wp;
    int texW = 120, texH = 120;
    ResourceManager::getInstance().getTextureDimensions(r, wpPath, texW, texH);
    if (texW <= 0) texW = 120;
    if (texH <= 0) texH = 120;

    for (int y = 0; y < h; y += texH) {
        for (int x = 0; x < w; x += texW) {
            ResourceManager::getInstance().drawTexture(r, wpPath, x, y, texW, texH);
        }
    }
}

void GameRoom::renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT, state.stats.currentWallpaper);

    // Ball on TV
    m_ball.render(renderer, state.stats.currentBall, 2.25f);

    // Joystick console on TV
    ResourceManager::getInstance().drawTexture(renderer, "images/room/joystick.png", 1620, 680, 180, 180);

    pou.render(renderer, 960.0f, 620.0f, 3.2f, true);
}

void GameRoom::renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT, state.stats.currentWallpaper);

    // Games Joystick button
    ResourceManager::getInstance().drawTextureCentered(renderer, "images/room/joystick.png", static_cast<int>(m_gamesBtnX), static_cast<int>(m_gamesBtnY), 90, 90);
    ResourceManager::getInstance().renderText(renderer, "Games", static_cast<int>(m_gamesBtnX), static_cast<int>(m_gamesBtnY + 58), 18, {100, 80, 60, 255}, true);

    // Render interactive Pou
    pou.render(renderer, 427.0f, 250.0f, 1.4f, false);

    // Render Ball
    m_ball.render(renderer, state.stats.currentBall, 1.0f);

    // Mini-Games selection modal
    if (m_showGameMenu) {
        renderGameMenu(renderer);
    }
}

void GameRoom::renderGameMenu(SDL_Renderer* r) {
    // Dark modal overlay
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
    SDL_Rect screenRect = {0, 0, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT};
    SDL_RenderFillRect(r, &screenRect);

    // Dialog card
    SDL_Rect cardRect = {120, 50, 614, 380};
    ResourceManager::fillRoundedRect(r, cardRect, 22, {252, 246, 235, 255});
    SDL_SetRenderDrawColor(r, 180, 150, 110, 255);
    SDL_RenderDrawRect(r, &cardRect);

    // Title
    ResourceManager::getInstance().renderText(r, "Mini-Games", 427, 85, 32, {80, 50, 30, 255}, true);

    // Close button (X)
    ResourceManager::getInstance().renderText(r, "[X]", 695, 75, 24, {180, 60, 60, 255}, true);

    // 4 Games options cards
    const char* gameTitles[] = {"Food Drop", "Sky Fall", "Pou Jump", "Pou Match"};
    const char* gameSubtitles[] = {"Catch food & dodge trash", "Dodge clouds & dive", "Bounce up platforms", "Match-3 color puzzle"};
    const char* gameIcons[] = {"images/games/food/option.png", "images/games/fall/option.png", "images/games/jump/option.png", "images/games/match/option.png"};

    for (int i = 0; i < 4; ++i) {
        int row = i / 2;
        int col = i % 2;
        int x = 150 + col * 280;
        int y = 135 + row * 135;

        SDL_Rect itemRect = {x, y, 260, 115};
        ResourceManager::fillRoundedRect(r, itemRect, 14, {240, 230, 210, 255});
        SDL_SetRenderDrawColor(r, 200, 180, 150, 255);
        SDL_RenderDrawRect(r, &itemRect);

        // Icon
        ResourceManager::getInstance().drawTexture(r, gameIcons[i], x + 15, y + 20, 75, 75);

        // Text
        ResourceManager::getInstance().renderText(r, gameTitles[i], x + 100, y + 30, 22, {70, 45, 25, 255});
        ResourceManager::getInstance().renderText(r, gameSubtitles[i], x + 100, y + 65, 14, {120, 100, 80, 255});
    }
}

void GameRoom::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) {
    if (m_showGameMenu) {
        if (justPressed) {
            // Close button
            if (touchX >= 665 && touchX <= 725 && touchY >= 55 && touchY <= 100) {
                m_showGameMenu = false;
                ResourceManager::getInstance().playSound("click.wav", 64);
                return;
            }

            // Click on game cards
            for (int i = 0; i < 4; ++i) {
                int row = i / 2;
                int col = i % 2;
                int x = 150 + col * 280;
                int y = 135 + row * 135;
                if (touchX >= x && touchX <= x + 260 && touchY >= y && touchY <= y + 115) {
                    m_showGameMenu = false;
                    ResourceManager::getInstance().playSound("click.wav", 80);
                    switch (i) {
                        case 0: m_requestedGame = PouConfig::GameId::FOOD_DROP; break;
                        case 1: m_requestedGame = PouConfig::GameId::SKY_FALL; break;
                        case 2: m_requestedGame = PouConfig::GameId::POU_JUMP; break;
                        case 3: m_requestedGame = PouConfig::GameId::POU_MATCH; break;
                    }
                    return;
                }
            }
        }
        return;
    }

    // Normal game room interaction
    if (justPressed) {
        // Joystick button
        float distToJoystick = std::sqrt(std::pow(touchX - m_gamesBtnX, 2) + std::pow(touchY - m_gamesBtnY, 2));
        if (distToJoystick <= 50.0f) {
            m_showGameMenu = true;
            ResourceManager::getInstance().playSound("click.wav", 80);
            return;
        }

        // Petting Pou
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            pou.triggerPetting();
            state.playPetting(0.2f);
        }
    }

    // Ball touch / flick handling
    m_ball.handleTouch(touchX, touchY, justPressed, isTouched, justReleased, state);

    if (isTouched && !m_showGameMenu) {
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            state.playPetting(PouConfig::FIXED_DELTA_TIME);
        }
    }
}
