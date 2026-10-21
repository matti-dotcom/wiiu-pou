// src/games/FoodGame.hpp
#pragma once

#include "MiniGame.hpp"
#include <vector>
#include <string>

struct FallingItem {
    float x, y;
    float vy;
    bool isBad;
    std::string iconPath;
    float size;
};

class FoodGame : public MiniGame {
public:
    FoodGame();

    void start(PouState& state) override;
    void update(float dt, const InputState& input, PouState& state) override;
    void renderTV(SDL_Renderer* renderer, const PouState& state) override;
    void renderDRC(SDL_Renderer* renderer, const PouState& state) override;

    bool isGameOver() const override { return m_gameOver; }
    bool wantsExit() const override { return m_wantsExit; }
    int getScore() const override { return m_score; }
    int getCoinsEarned() const override { return m_coinsEarned; }
    std::string getGameName() const override { return "Food Drop"; }

private:
    float m_pouX = 427.0f;
    float m_pouY = 410.0f;
    float m_pouSpeed = 480.0f;

    std::vector<FallingItem> m_items;
    float m_spawnTimer = 0.0f;
    float m_spawnInterval = 1.1f;

    int m_score = 0;
    int m_coinsEarned = 0;
    int m_misses = 0;
    const int MAX_MISSES = 3;

    bool m_gameOver = false;
    bool m_wantsExit = false;

    void spawnItem();
    void renderGameElements(SDL_Renderer* r, float scale, bool isTV);
};
