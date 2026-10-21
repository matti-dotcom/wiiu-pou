// src/rooms/KitchenRoom.hpp
#pragma once

#include "Room.hpp"
#include <vector>

class KitchenRoom : public Room {
public:
    KitchenRoom();

    void enter(PouState& state, Pou& pou) override;
    void update(float dt, PouState& state, Pou& pou) override;
    void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) override;

    std::string getRoomName() const override { return "Kitchen"; }

private:
    int m_selectedFoodIndex = 0;
    std::vector<std::string> m_availableFoods;

    // Food drag and drop state
    bool m_isDraggingFood = false;
    float m_dragFoodX = 0.0f;
    float m_dragFoodY = 0.0f;
    float m_trayFoodX = 427.0f;
    float m_trayFoodY = 415.0f;

    void updateAvailableFoods(PouState& state);
    void drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp);
};
