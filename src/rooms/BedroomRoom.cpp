// src/rooms/BedroomRoom.cpp
#include "BedroomRoom.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>
#include <cstdlib>

BedroomRoom::BedroomRoom() {
    m_zzzTimer = 0.0f;
}

void BedroomRoom::enter(PouState& state, Pou& pou) {
    m_zzzs.clear();
}

void BedroomRoom::update(float dt, PouState& state, Pou& pou) {
    if (state.stats.isAsleep) {
        m_zzzTimer += dt;
        if (m_zzzTimer >= 0.8f) {
            m_zzzTimer = 0.0f;
            SleepZzz z;
            z.x = 427.0f + (rand() % 40) - 20.0f;
            z.y = 190.0f;
            z.vy = 30.0f + (rand() % 20);
            z.alpha = 1.0f;
            z.size = 18.0f + (rand() % 10);
            m_zzzs.push_back(z);
        }

        for (auto it = m_zzzs.begin(); it != m_zzzs.end();) {
            it->y -= it->vy * dt;
            it->alpha -= dt * 0.4f;
            if (it->alpha <= 0.0f) {
                it = m_zzzs.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        m_zzzs.clear();
    }
}

void BedroomRoom::drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp) {
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

void BedroomRoom::drawZzzs(SDL_Renderer* r, float scale) {
    for (const auto& z : m_zzzs) {
        SDL_Color color = {220, 240, 255, static_cast<uint8_t>(z.alpha * 255)};
        ResourceManager::getInstance().renderText(r, "Z", static_cast<int>(z.x * scale), static_cast<int>(z.y * scale), static_cast<int>(z.size * scale), color, true);
    }
}

void BedroomRoom::renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT, state.stats.currentWallpaper);

    // Large closet and lamp on TV
    ResourceManager::getInstance().drawTexture(renderer, "images/closets/closet.png", 220, 520, 240, 360);

    std::string lampTex = state.stats.isAsleep ? "images/lamps/1_yellow_blue_off.png" : "images/lamps/1_yellow_blue_on.png";
    ResourceManager::getInstance().drawTexture(renderer, lampTex, 1500, 560, 200, 320);

    pou.render(renderer, 960.0f, 620.0f, 3.2f, true);

    if (state.stats.isAsleep) {
        // Darkness overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 15, 15, 40, 210);
        SDL_Rect tvScreen = {0, 0, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT};
        SDL_RenderFillRect(renderer, &tvScreen);

        drawZzzs(renderer, 2.25f);
    }
}

void BedroomRoom::renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT, state.stats.currentWallpaper);

    // Closet
    ResourceManager::getInstance().drawTextureCentered(renderer, "images/closets/closet.png", static_cast<int>(m_closetX), static_cast<int>(m_closetY), 120, 180);
    ResourceManager::getInstance().renderText(renderer, "Closet", static_cast<int>(m_closetX), static_cast<int>(m_closetY + 105), 18, {100, 80, 60, 255}, true);

    // Lamp
    std::string lampTex = state.stats.isAsleep ? "images/lamps/1_yellow_blue_off.png" : "images/lamps/1_yellow_blue_on.png";
    ResourceManager::getInstance().drawTextureCentered(renderer, lampTex, static_cast<int>(m_lampX), static_cast<int>(m_lampY), 100, 160);
    ResourceManager::getInstance().renderText(renderer, state.stats.isAsleep ? "Turn On" : "Sleep", static_cast<int>(m_lampX), static_cast<int>(m_lampY + 95), 18, {100, 80, 60, 255}, true);

    // Render interactive Pou
    pou.render(renderer, 427.0f, 250.0f, 1.4f, false);

    if (state.stats.isAsleep) {
        // Darkness overlay on GamePad
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 15, 15, 40, 195);
        SDL_Rect drcScreen = {0, 0, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT};
        SDL_RenderFillRect(renderer, &drcScreen);

        // Keep lamp visible through darkness
        ResourceManager::getInstance().drawTextureCentered(renderer, "images/lamps/1_yellow_blue_off.png", static_cast<int>(m_lampX), static_cast<int>(m_lampY), 100, 160);
        ResourceManager::getInstance().renderText(renderer, "Turn On", static_cast<int>(m_lampX), static_cast<int>(m_lampY + 95), 18, {220, 220, 255, 255}, true);

        drawZzzs(renderer, 1.0f);
    }
}

void BedroomRoom::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) {
    if (justPressed) {
        // Lamp tap (toggle sleep)
        float distToLamp = std::sqrt(std::pow(touchX - m_lampX, 2) + std::pow(touchY - m_lampY, 2));
        if (distToLamp <= 60.0f) {
            state.toggleSleep();
            ResourceManager::getInstance().playSound("click.wav", 80);
            return;
        }

        // Closet tap
        if (!state.stats.isAsleep) {
            float distToCloset = std::sqrt(std::pow(touchX - m_closetX, 2) + std::pow(touchY - m_closetY, 2));
            if (distToCloset <= 60.0f) {
                // Cycle wallpaper for instant wardrobe preview!
                static const std::vector<std::string> wpList = {"111.png", "112.png", "121.png", "131.png", "141.png", "151.png", "161.png", "171.png"};
                static int wpIdx = 0;
                wpIdx = (wpIdx + 1) % wpList.size();
                state.stats.currentWallpaper = wpList[wpIdx];
                ResourceManager::getInstance().playSound("buy.wav", 64);
                return;
            }

            // Petting Pou
            if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
                pou.triggerPetting();
                state.playPetting(0.2f);
            }
        }
    }

    if (!state.stats.isAsleep && isTouched) {
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            state.playPetting(PouConfig::FIXED_DELTA_TIME);
        }
    }
}
