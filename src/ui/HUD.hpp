// src/ui/HUD.hpp
#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "../core/PouState.hpp"

class HUD {
public:
    HUD() = default;

    void renderTV(SDL_Renderer* renderer, const PouState& state, const std::string& roomTitle);
    void renderDRC(SDL_Renderer* renderer, const PouState& state, const std::string& roomTitle);

    // Returns true if an action was handled (room change or shop open)
    bool handleTouch(int touchX, int touchY, bool justPressed, PouState& state, int& outRoomChange, bool& outOpenShop);

private:
    void renderGaugeCircleTV(SDL_Renderer* r, int cx, int cy, int radius, float value, const std::string& iconPath, const std::string& label, SDL_Color color);
};
