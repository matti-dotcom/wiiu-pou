// src/rooms/LaboratoryRoom.hpp
#pragma once

#include "Room.hpp"
#include <vector>

class LaboratoryRoom : public Room {
public:
    LaboratoryRoom();

    void enter(PouState& state, Pou& pou) override;
    void update(float dt, PouState& state, Pou& pou) override;
    void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) override;

    std::string getRoomName() const override { return "Laboratory"; }

private:
    int m_selectedPotionIndex = 0;
    std::vector<PouConfig::PotionType> m_availablePotions;

    bool m_isDraggingPotion = false;
    float m_dragPotionX = 0.0f;
    float m_dragPotionY = 0.0f;
    const float m_trayPotionX = 427.0f;
    const float m_trayPotionY = 415.0f;

    void updateAvailablePotions(PouState& state);
    std::string getPotionTextureName(PouConfig::PotionType type) const;
    std::string getPotionDisplayName(PouConfig::PotionType type) const;
    void drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp);
};
