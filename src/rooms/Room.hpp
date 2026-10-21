// src/rooms/Room.hpp
#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "../core/PouState.hpp"
#include "../entities/Pou.hpp"

class Room {
public:
    virtual ~Room() = default;

    virtual void enter(PouState& state, Pou& pou) {}
    virtual void exit(PouState& state, Pou& pou) {}

    virtual void update(float dt, PouState& state, Pou& pou) = 0;
    virtual void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) = 0;
    virtual void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) = 0;
    virtual void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) = 0;

    virtual std::string getRoomName() const = 0;
};
