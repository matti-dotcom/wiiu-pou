// src/games/JumpGame.hpp
#pragma once

#include "MiniGame.hpp"
#include <vector>
#include <string>

struct JumpPlatform {
    float x, y;
    float w, h;
    bool isMoving;
    float vx;
    bool hasCoin;
    bool coinCollected;
    std::string texPath;
};

class JumpGame : public MiniGame {
public:
    JumpGame();

    void start(PouState& state) override;
    void update(float dt, const InputState& input, PouState& state) override;
    void renderTV(SDL_Renderer* renderer, const PouState& state) override;
    void renderDRC(SDL_Renderer* renderer, const PouState& state) override;

    bool isGameOver() const override { return m_gameOver; }
    bool wantsExit() const override { return m_wantsExit; }
    int getScore() const override { return m_score; }
    int getCoinsEarned() const override { return m_coinsEarned; }
    std::string getGameName() const override { return "Pou Jump"; }

private:
    float m_pouX = 427.0f;
    float m_pouY = 350.0f;
    float m_pouVy = -680.0f;
    float m_highestY = 0.0f;

    std::vector<JumpPlatform> m_platforms;
    float m_topmostPlatformY = 0.0f;

    int m_score = 0;
    int m_coinsEarned = 0;
    bool m_gameOver = false;
    bool m_wantsExit = false;

    void spawnPlatform(float y);
    void renderGameElements(SDL_Renderer* r, float scale, bool isTV);
};
