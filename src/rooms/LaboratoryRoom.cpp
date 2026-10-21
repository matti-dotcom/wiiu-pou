// src/rooms/LaboratoryRoom.cpp
#include "LaboratoryRoom.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

LaboratoryRoom::LaboratoryRoom() {
    m_isDraggingPotion = false;
    m_selectedPotionIndex = 0;
}

void LaboratoryRoom::enter(PouState& state, Pou& pou) {
    updateAvailablePotions(state);
    m_isDraggingPotion = false;
}

void LaboratoryRoom::updateAvailablePotions(PouState& state) {
    m_availablePotions.clear();
    for (const auto& pair : state.stats.potions) {
        if (pair.second > 0) {
            m_availablePotions.push_back(static_cast<PouConfig::PotionType>(pair.first));
        }
    }
    if (m_availablePotions.empty()) {
        m_selectedPotionIndex = 0;
    } else if (m_selectedPotionIndex >= static_cast<int>(m_availablePotions.size())) {
        m_selectedPotionIndex = 0;
    }
}

std::string LaboratoryRoom::getPotionTextureName(PouConfig::PotionType type) const {
    switch (type) {
        case PouConfig::PotionType::HEALTH: return "Health Potion.png";
        case PouConfig::PotionType::SMALL_HEALTH: return "Small Health.png";
        case PouConfig::PotionType::ENERGIZER: return "Energizer.png";
        case PouConfig::PotionType::HUNGER: return "Hunger Potion.png";
        case PouConfig::PotionType::FAT_BURNER: return "Fat Burner.png";
        case PouConfig::PotionType::BABY: return "Baby Potion.png";
        case PouConfig::PotionType::ADULT: return "Adult Potion.png";
        default: return "Health Potion.png";
    }
}

std::string LaboratoryRoom::getPotionDisplayName(PouConfig::PotionType type) const {
    switch (type) {
        case PouConfig::PotionType::HEALTH: return "Health Potion";
        case PouConfig::PotionType::SMALL_HEALTH: return "Small Health";
        case PouConfig::PotionType::ENERGIZER: return "Energizer";
        case PouConfig::PotionType::HUNGER: return "Hunger Potion";
        case PouConfig::PotionType::FAT_BURNER: return "Fat Burner";
        case PouConfig::PotionType::BABY: return "Baby Potion";
        case PouConfig::PotionType::ADULT: return "Adult Potion";
        default: return "Potion";
    }
}

void LaboratoryRoom::update(float dt, PouState& state, Pou& pou) {
    updateAvailablePotions(state);

    if (m_isDraggingPotion) {
        float dx = (m_dragPotionX - 427.0f) * 0.08f;
        float dy = (m_dragPotionY - 260.0f) * 0.08f;
        pou.setTargetLook(dx, dy);

        float distToMouth = std::sqrt(std::pow(m_dragPotionX - 427.0f, 2) + std::pow(m_dragPotionY - 280.0f, 2));
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

void LaboratoryRoom::drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp) {
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

void LaboratoryRoom::renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT, state.stats.currentWallpaper);

    // Lab Shelf on TV
    ResourceManager::getInstance().drawTexture(renderer, "images/shelves/1_beige.png", 100, 720, 240, 100);

    pou.render(renderer, 960.0f, 620.0f, 3.2f, true);
}

void LaboratoryRoom::renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT, state.stats.currentWallpaper);

    // Shelf drawer background
    SDL_Rect trayRect = {140, 365, 574, 95};
    ResourceManager::fillRoundedRect(renderer, trayRect, 18, {255, 255, 255, 220});
    SDL_SetRenderDrawColor(renderer, 200, 180, 150, 255);
    SDL_RenderDrawRect(renderer, &trayRect);

    // Shelf icon
    ResourceManager::getInstance().drawTexture(renderer, "images/shelves/1_beige.png", 160, 385, 80, 45);

    // Render interactive Pou
    pou.render(renderer, 427.0f, 250.0f, 1.4f, false);

    // Potion tray
    if (!m_availablePotions.empty() && m_selectedPotionIndex < static_cast<int>(m_availablePotions.size())) {
        PouConfig::PotionType currentP = m_availablePotions[m_selectedPotionIndex];
        std::string texName = getPotionTextureName(currentP);
        int count = state.stats.potions[static_cast<int>(currentP)];

        ResourceManager::getInstance().drawTexture(renderer, "images/view/previous.png", 320, 395, 30, 35);
        ResourceManager::getInstance().drawTexture(renderer, "images/view/next.png", 510, 395, 30, 35);

        if (!m_isDraggingPotion) {
            ResourceManager::getInstance().drawTextureCentered(renderer, "images/potions/" + texName, static_cast<int>(m_trayPotionX), static_cast<int>(m_trayPotionY), 60, 60);
        } else {
            ResourceManager::getInstance().drawTextureCentered(renderer, "images/potions/" + texName, static_cast<int>(m_dragPotionX), static_cast<int>(m_dragPotionY), 70, 70);
        }

        std::string countStr = "x" + std::to_string(count);
        ResourceManager::getInstance().renderText(renderer, countStr, 470, 420, 22, {80, 60, 40, 255});
    } else {
        ResourceManager::getInstance().renderText(renderer, "Shelf is empty! Buy potions in Shop", 427, 410, 22, {140, 100, 80, 255}, true);
    }
}

void LaboratoryRoom::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) {
    if (m_availablePotions.empty()) return;

    if (justPressed) {
        // Left arrow
        if (touchX >= 310 && touchX <= 355 && touchY >= 385 && touchY <= 440) {
            m_selectedPotionIndex = (m_selectedPotionIndex - 1 + static_cast<int>(m_availablePotions.size())) % m_availablePotions.size();
            ResourceManager::getInstance().playSound("click.wav", 64);
            return;
        }
        // Right arrow
        if (touchX >= 500 && touchX <= 545 && touchY >= 385 && touchY <= 440) {
            m_selectedPotionIndex = (m_selectedPotionIndex + 1) % m_availablePotions.size();
            ResourceManager::getInstance().playSound("click.wav", 64);
            return;
        }

        // Tap potion to drag
        float distToTray = std::sqrt(std::pow(touchX - m_trayPotionX, 2) + std::pow(touchY - m_trayPotionY, 2));
        if (distToTray <= 45.0f) {
            m_isDraggingPotion = true;
            m_dragPotionX = static_cast<float>(touchX);
            m_dragPotionY = static_cast<float>(touchY);
            return;
        }

        // Petting Pou
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            pou.triggerPetting();
            state.playPetting(0.2f);
        }
    }

    if (m_isDraggingPotion) {
        if (isTouched) {
            m_dragPotionX = static_cast<float>(touchX);
            m_dragPotionY = static_cast<float>(touchY);
        } else if (justReleased) {
            m_isDraggingPotion = false;

            float distToMouth = std::sqrt(std::pow(m_dragPotionX - 427.0f, 2) + std::pow(m_dragPotionY - 280.0f, 2));
            if (distToMouth < 65.0f) {
                PouConfig::PotionType pType = m_availablePotions[m_selectedPotionIndex];
                if (state.usePotion(pType)) {
                    ResourceManager::getInstance().playSound("potion.wav", 110);
                    pou.triggerEating();
                }
            }
        }
    } else if (isTouched) {
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            state.playPetting(PouConfig::FIXED_DELTA_TIME);
        }
    }
}
