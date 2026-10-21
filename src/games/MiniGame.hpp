// src/games/MiniGame.hpp
#pragma once

#include <SDL2/SDL.h>
#include "../core/Config.hpp"
#include "../core/PouState.hpp"
#include "../core/InputManager.hpp"
#include <string>

class MiniGame {
public:
    virtual ~MiniGame() = default;

    virtual void start(PouState& state) = 0;
    virtual void update(float dt, const InputState& input, PouState& state) = 0;
    virtual void renderTV(SDL_Renderer* renderer, const PouState& state) = 0;
    virtual void renderDRC(SDL_Renderer* renderer, const PouState& state) = 0;

    virtual bool isGameOver() const = 0;
    virtual bool wantsExit() const = 0;
    virtual int getScore() const = 0;
    virtual int getCoinsEarned() const = 0;
    virtual std::string getGameName() const = 0;
};
