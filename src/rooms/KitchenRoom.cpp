// src/rooms/KitchenRoom.cpp
#include "KitchenRoom.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

KitchenRoom::KitchenRoom() {
    m_isDraggingFood = false;
    m_selectedFoodIndex = 0;
}

void KitchenRoom::enter(PouState& state, Pou& pou) {
    updateAvailableFoods(state);
    m_isDraggingFood = false;
}

void KitchenRoom::updateAvailableFoods(PouState& state) {
    m_availableFoods.clear();
    for (const auto& pair : state.stats.foods) {
        if (pair.second > 0) {
            m_availableFoods.push_back(pair.first);
        }
    }
    if (m_availableFoods.empty()) {
        m_selectedFoodIndex = 0;
    } else if (m_selectedFoodIndex >= static_cast<int>(m_availableFoods.size())) {
        m_selectedFoodIndex = 0;
    }
}

void KitchenRoom::update(float dt, PouState& state, Pou& pou) {
    updateAvailableFoods(state);

    if (m_isDraggingFood) {
        // Pou tracks food with eyes
        float dx = (m_dragFoodX - 427.0f) * 0.08f;
        float dy = (m_dragFoodY - 260.0f) * 0.08f;
        pou.setTargetLook(dx, dy);

        // Distance to Pou mouth (mouth is around 427, 280)
        float distToMouth = std::sqrt(std::pow(m_dragFoodX - 427.0f, 2) + std::pow(m_dragFoodY - 280.0f, 2));
        if (distToMouth < 65.0f) {
            pou.openMouthForFood(true);
        } else {
            pou.openMouthForFood(false);
        }
    } else {
        pou.clearTargetLook();
        pou.openMouthForFood(false);
    }
}

void KitchenRoom::drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp) {
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

void KitchenRoom::renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    // 1080p TV View
    drawWallpaper(renderer, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT, state.stats.currentWallpaper);

    // Ambient Fridge in corner
    ResourceManager::getInstance().drawTexture(renderer, "images/fridges/fridge.png", 80, 720, 160, 240);

    // Render big Pou on TV
    pou.render(renderer, 960.0f, 620.0f, 3.2f, true);
}

void KitchenRoom::renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    // 480p GamePad View
    drawWallpaper(renderer, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT, state.stats.currentWallpaper);

    // Fridge drawer background tray
    SDL_Rect trayRect = {140, 365, 574, 95};
    ResourceManager::fillRoundedRect(renderer, trayRect, 18, {255, 255, 255, 220});
    SDL_SetRenderDrawColor(renderer, 200, 180, 150, 255);
    SDL_RenderDrawRect(renderer, &trayRect);

    // Left Fridge icon
    ResourceManager::getInstance().drawTexture(renderer, "images/fridges/fridge.png", 160, 375, 60, 80);

    // Render interactive Pou
    pou.render(renderer, 427.0f, 250.0f, 1.4f, false);

    // Food selection tray
    if (!m_availableFoods.empty() && m_selectedFoodIndex < static_cast<int>(m_availableFoods.size())) {
        std::string currentFood = m_availableFoods[m_selectedFoodIndex];
        int count = state.stats.foods[currentFood];

        // Previous / Next arrows
        ResourceManager::getInstance().drawTexture(renderer, "images/view/previous.png", 320, 395, 30, 35);
        ResourceManager::getInstance().drawTexture(renderer, "images/view/next.png", 510, 395, 30, 35);

        // Food item icon (if not dragging, render on tray; if dragging, render at touch)
        if (!m_isDraggingFood) {
            ResourceManager::getInstance().drawTextureCentered(renderer, "images/food/" + currentFood, static_cast<int>(m_trayFoodX), static_cast<int>(m_trayFoodY), 65, 65);
        } else {
            ResourceManager::getInstance().drawTextureCentered(renderer, "images/food/" + currentFood, static_cast<int>(m_dragFoodX), static_cast<int>(m_dragFoodY), 75, 75);
        }

        // Count label
        std::string countStr = "x" + std::to_string(count);
        ResourceManager::getInstance().renderText(renderer, countStr, 470, 420, 22, {80, 60, 40, 255});
    } else {
        ResourceManager::getInstance().renderText(renderer, "Fridge is empty! Buy food in Shop", 427, 410, 22, {140, 100, 80, 255}, true);
    }
}

void KitchenRoom::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) {
    if (m_availableFoods.empty()) return;

    // Arrows tap
    if (justPressed) {
        // Left arrow (310 - 350, 385 - 435)
        if (touchX >= 310 && touchX <= 355 && touchY >= 385 && touchY <= 440) {
            m_selectedFoodIndex = (m_selectedFoodIndex - 1 + static_cast<int>(m_availableFoods.size())) % m_availableFoods.size();
            ResourceManager::getInstance().playSound("click.wav", 64);
            return;
        }
        // Right arrow (500 - 545, 385 - 440)
        if (touchX >= 500 && touchX <= 545 && touchY >= 385 && touchY <= 440) {
            m_selectedFoodIndex = (m_selectedFoodIndex + 1) % m_availableFoods.size();
            ResourceManager::getInstance().playSound("click.wav", 64);
            return;
        }

        // Tap food on tray to start dragging
        float distToTray = std::sqrt(std::pow(touchX - m_trayFoodX, 2) + std::pow(touchY - m_trayFoodY, 2));
        if (distToTray <= 45.0f) {
            m_isDraggingFood = true;
            m_dragFoodX = static_cast<float>(touchX);
            m_dragFoodY = static_cast<float>(touchY);
            return;
        }

        // Petting Pou
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            pou.triggerPetting();
            state.playPetting(0.2f);
        }
    }

    if (m_isDraggingFood) {
        if (isTouched) {
            m_dragFoodX = static_cast<float>(touchX);
            m_dragFoodY = static_cast<float>(touchY);
        } else if (justReleased) {
            m_isDraggingFood = false;

            // Check if dropped into mouth
            float distToMouth = std::sqrt(std::pow(m_dragFoodX - 427.0f, 2) + std::pow(m_dragFoodY - 280.0f, 2));
            if (distToMouth < 65.0f) {
                if (state.canFeed()) {
                    std::string food = m_availableFoods[m_selectedFoodIndex];
                    state.feed(18.0f, food);
                    pou.triggerEating();
                } else {
                    pou.triggerSayingNo();
                }
            }
        }
    } else if (isTouched) {
        // Continuous petting while touching Pou
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            state.playPetting(PouConfig::FIXED_DELTA_TIME);
        }
    }
}
