// src/rooms/BathroomRoom.cpp
#include "BathroomRoom.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

BathroomRoom::BathroomRoom() {
    m_isDraggingSoap = false;
    m_isDraggingShower = false;
}

void BathroomRoom::enter(PouState& state, Pou& pou) {
    m_isDraggingSoap = false;
    m_isDraggingShower = false;
    m_soapX = m_soapTrayX;
    m_soapY = m_soapTrayY;
    m_showerX = m_showerTrayX;
    m_showerY = m_showerTrayY;
}

void BathroomRoom::update(float dt, PouState& state, Pou& pou) {
    if (m_isDraggingSoap) {
        float dx = (m_soapX - 427.0f) * 0.08f;
        float dy = (m_soapY - 260.0f) * 0.08f;
        pou.setTargetLook(dx, dy);

        // Check if rubbing Pou
        if (pou.isPointInside(m_soapX, m_soapY, 427.0f, 250.0f, 1.4f)) {
            state.rubSoap(dt * 30.0f);
            pou.triggerPetting();
        }
    } else if (m_isDraggingShower) {
        float dx = (m_showerX - 427.0f) * 0.08f;
        float dy = (m_showerY - 260.0f) * 0.08f;
        pou.setTargetLook(dx, dy);

        // Check if rinsing Pou
        if (pou.isPointInside(m_showerX, m_showerY, 427.0f, 250.0f, 1.4f)) {
            state.rinseShower(dt * 20.0f);
        }
    } else {
        pou.clearTargetLook();
    }
}

void BathroomRoom::drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp) {
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

void BathroomRoom::renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT, state.stats.currentWallpaper);

    // Large soap and shower on TV
    ResourceManager::getInstance().drawTexture(renderer, "images/soaps/blue.png", 180, 780, 160, 120);
    ResourceManager::getInstance().drawTexture(renderer, "images/shelves/1_beige.png", 1600, 780, 180, 80);

    pou.render(renderer, 960.0f, 620.0f, 3.2f, true);
}

void BathroomRoom::renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) {
    drawWallpaper(renderer, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT, state.stats.currentWallpaper);

    // Bottom tray for bathroom tools
    SDL_Rect trayRect = {160, 365, 534, 95};
    ResourceManager::fillRoundedRect(renderer, trayRect, 18, {255, 255, 255, 220});
    SDL_SetRenderDrawColor(renderer, 200, 180, 150, 255);
    SDL_RenderDrawRect(renderer, &trayRect);

    // Render interactive Pou
    pou.render(renderer, 427.0f, 250.0f, 1.4f, false);

    // Draw Soap
    ResourceManager::getInstance().drawTextureCentered(renderer, "images/soaps/blue.png", static_cast<int>(m_soapX), static_cast<int>(m_soapY), 85, 65);
    ResourceManager::getInstance().renderText(renderer, "Soap", static_cast<int>(m_soapTrayX), 445, 18, {100, 80, 60, 255}, true);

    // Draw Shower / Water Rinsing tool
    ResourceManager::getInstance().drawTextureCentered(renderer, "images/soaps/bubble.png", static_cast<int>(m_showerX), static_cast<int>(m_showerY), 70, 70);
    ResourceManager::getInstance().renderText(renderer, "Shower", static_cast<int>(m_showerTrayX), 445, 18, {100, 80, 60, 255}, true);

    // If dragging shower, draw water spray lines
    if (m_isDraggingShower) {
        SDL_SetRenderDrawColor(renderer, 100, 180, 255, 200);
        for (int i = -3; i <= 3; ++i) {
            SDL_RenderDrawLine(renderer, static_cast<int>(m_showerX), static_cast<int>(m_showerY + 25),
                                         static_cast<int>(m_showerX + i * 12), static_cast<int>(m_showerY + 70));
        }
    }
}

void BathroomRoom::handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) {
    if (justPressed) {
        // Tap soap to drag
        float distToSoap = std::sqrt(std::pow(touchX - m_soapX, 2) + std::pow(touchY - m_soapY, 2));
        if (distToSoap <= 45.0f) {
            m_isDraggingSoap = true;
            m_soapX = static_cast<float>(touchX);
            m_soapY = static_cast<float>(touchY);
            return;
        }

        // Tap shower to drag
        float distToShower = std::sqrt(std::pow(touchX - m_showerX, 2) + std::pow(touchY - m_showerY, 2));
        if (distToShower <= 45.0f) {
            m_isDraggingShower = true;
            m_showerX = static_cast<float>(touchX);
            m_showerY = static_cast<float>(touchY);
            return;
        }

        // Petting Pou
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            pou.triggerPetting();
            state.playPetting(0.2f);
        }
    }

    if (m_isDraggingSoap) {
        if (isTouched) {
            m_soapX = static_cast<float>(touchX);
            m_soapY = static_cast<float>(touchY);
        } else if (justReleased) {
            m_isDraggingSoap = false;
            m_soapX = m_soapTrayX;
            m_soapY = m_soapTrayY;
        }
    } else if (m_isDraggingShower) {
        if (isTouched) {
            m_showerX = static_cast<float>(touchX);
            m_showerY = static_cast<float>(touchY);
        } else if (justReleased) {
            m_isDraggingShower = false;
            m_showerX = m_showerTrayX;
            m_showerY = m_showerTrayY;
        }
    } else if (isTouched) {
        if (pou.isPointInside(static_cast<float>(touchX), static_cast<float>(touchY), 427.0f, 250.0f, 1.4f)) {
            state.playPetting(PouConfig::FIXED_DELTA_TIME);
        }
    }
}
