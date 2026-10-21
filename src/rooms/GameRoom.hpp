// src/rooms/GameRoom.hpp
#pragma once

#include "Room.hpp"
#include "../entities/Ball.hpp"

class GameRoom : public Room {
public:
    GameRoom();

    void enter(PouState& state, Pou& pou) override;
    void update(float dt, PouState& state, Pou& pou) override;
    void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) override;

    std::string getRoomName() const override { return "Game Room"; }

    // Game selection state
    bool isGameMenuOpen() const { return m_showGameMenu; }
    void closeGameMenu() { m_showGameMenu = false; }
    PouConfig::GameId getRequestedGame() const { return m_requestedGame; }
    void clearRequestedGame() { m_requestedGame = PouConfig::GameId::NONE; }

private:
    Ball m_ball;
    bool m_showGameMenu = false;
    PouConfig::GameId m_requestedGame = PouConfig::GameId::NONE;

    float m_gamesBtnX = 720.0f;
    float m_gamesBtnY = 250.0f;

    void drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp);
    void renderGameMenu(SDL_Renderer* r);
};
