// src/games/MatchGame.hpp
#pragma once

#include "MiniGame.hpp"
#include <vector>

constexpr int MATCH_GRID_COLS = 7;
constexpr int MATCH_GRID_ROWS = 7;

struct MatchCell {
    int colorId; // 0-5
    bool matched;
    float animY; // For drop animation
};

class MatchGame : public MiniGame {
public:
    MatchGame();

    void start(PouState& state) override;
    void update(float dt, const InputState& input, PouState& state) override;
    void renderTV(SDL_Renderer* renderer, const PouState& state) override;
    void renderDRC(SDL_Renderer* renderer, const PouState& state) override;

    bool isGameOver() const override { return m_gameOver; }
    bool wantsExit() const override { return m_wantsExit; }
    int getScore() const override { return m_score; }
    int getCoinsEarned() const override { return m_coinsEarned; }
    std::string getGameName() const override { return "Pou Match"; }

private:
    MatchCell m_grid[MATCH_GRID_ROWS][MATCH_GRID_COLS];

    int m_selectedR = -1;
    int m_selectedC = -1;

    float m_timeLeft = 60.0f;
    int m_score = 0;
    int m_coinsEarned = 0;

    bool m_gameOver = false;
    bool m_wantsExit = false;

    // Grid layout coordinates
    const float m_gridStartX = 230.0f;
    const float m_gridStartY = 60.0f;
    const float m_cellSize = 56.0f;

    void initGrid();
    bool checkAndResolveMatches(PouState& state);
    void dropAndRefill();
    void swapCells(int r1, int c1, int r2, int c2);
    void renderGameElements(SDL_Renderer* r, float scale, bool isTV);
    void drawPouHead(SDL_Renderer* r, int cx, int cy, int size, int colorId);
};
