// src/games/FallGame.hpp
#pragma once

#include "MiniGame.hpp"
#include <vector>

struct FallCloud {
    float x, y;
    float w, h;
    bool hasCoin;
    bool coinCollected;
    float coinX, coinY;
};

class FallGame : public MiniGame {
public:
    FallGame();

    void start(PouState& state) override;
    void update(float dt, const InputState& input, PouState& state) override;
    void renderTV(SDL_Renderer* renderer, const PouState& state) override;
    void renderDRC(SDL_Renderer* renderer, const PouState& state) override;

    bool isGameOver() const override { return m_gameOver; }
    bool wantsExit() const override { return m_wantsExit; }
    int getScore() const override { return m_score; }
    int getCoinsEarned() const override { return m_coinsEarned; }
    std::string getGameName() const override { return "Sky Fall"; }

private:
    float m_pouX = 427.0f;
    float m_pouY = 130.0f;
    float m_fallSpeed = 260.0f;

    std::vector<FallCloud> m_clouds;
    float m_spawnTimer = 0.0f;
    float m_spawnInterval = 1.2f;

    int m_score = 0;
    int m_coinsEarned = 0;
    int m_hits = 0;
    const int MAX_HITS = 3;

    bool m_gameOver = false;
    bool m_wantsExit = false;

    void spawnCloudPair();
    void renderGameElements(SDL_Renderer* r, float scale, bool isTV);
};
