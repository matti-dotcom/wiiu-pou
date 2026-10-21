// src/games/MatchGame.cpp
#include "MatchGame.hpp"
#include "../core/ResourceManager.hpp"
#include <cstdlib>
#include <cmath>

static const SDL_Color MATCH_COLORS[] = {
    {199, 149, 90, 255},  // 0: Classic Brown
    {255, 120, 160, 255}, // 1: Pink
    {80, 180, 240, 255},  // 2: Blue
    {245, 215, 60, 255},  // 3: Yellow
    {120, 210, 80, 255},  // 4: Green
    {160, 140, 180, 255}  // 5: Purple
};

MatchGame::MatchGame() {
    m_gameOver = false;
    m_wantsExit = false;
}

void MatchGame::start(PouState& state) {
    m_timeLeft = 60.0f;
    m_score = 0;
    m_coinsEarned = 0;
    m_selectedR = -1;
    m_selectedC = -1;
    m_gameOver = false;
    m_wantsExit = false;

    initGrid();
    ResourceManager::getInstance().playMusic("music_match.wav", true);
}

void MatchGame::initGrid() {
    for (int r = 0; r < MATCH_GRID_ROWS; ++r) {
        for (int c = 0; c < MATCH_GRID_COLS; ++c) {
            int col;
            do {
                col = rand() % 5;
            } while ((c >= 2 && m_grid[r][c - 1].colorId == col && m_grid[r][c - 2].colorId == col) ||
                     (r >= 2 && m_grid[r - 1][c].colorId == col && m_grid[r - 2][c].colorId == col));

            m_grid[r][c].colorId = col;
            m_grid[r][c].matched = false;
            m_grid[r][c].animY = 0.0f;
        }
    }
}

void MatchGame::swapCells(int r1, int c1, int r2, int c2) {
    int temp = m_grid[r1][c1].colorId;
    m_grid[r1][c1].colorId = m_grid[r2][c2].colorId;
    m_grid[r2][c2].colorId = temp;
}

bool MatchGame::checkAndResolveMatches(PouState& state) {
    bool found = false;

    // Reset matched flags
    for (int r = 0; r < MATCH_GRID_ROWS; ++r) {
        for (int c = 0; c < MATCH_GRID_COLS; ++c) {
            m_grid[r][c].matched = false;
        }
    }

    // Horizontal check
    for (int r = 0; r < MATCH_GRID_ROWS; ++r) {
        for (int c = 0; c < MATCH_GRID_COLS - 2; ++c) {
            int colId = m_grid[r][c].colorId;
            if (colId != -1 && m_grid[r][c + 1].colorId == colId && m_grid[r][c + 2].colorId == colId) {
                m_grid[r][c].matched = true;
                m_grid[r][c + 1].matched = true;
                m_grid[r][c + 2].matched = true;
                found = true;
            }
        }
    }

    // Vertical check
    for (int c = 0; c < MATCH_GRID_COLS; ++c) {
        for (int r = 0; r < MATCH_GRID_ROWS - 2; ++r) {
            int colId = m_grid[r][c].colorId;
            if (colId != -1 && m_grid[r + 1][c].colorId == colId && m_grid[r + 2][c].colorId == colId) {
                m_grid[r][c].matched = true;
                m_grid[r + 1][c].matched = true;
                m_grid[r + 2][c].matched = true;
                found = true;
            }
        }
    }

    if (found) {
        int matchCount = 0;
        for (int r = 0; r < MATCH_GRID_ROWS; ++r) {
            for (int c = 0; c < MATCH_GRID_COLS; ++c) {
                if (m_grid[r][c].matched) {
                    m_grid[r][c].colorId = -1; // Clear
                    matchCount++;
                }
            }
        }

        m_score += matchCount * 10;
        int coins = matchCount / 3;
        m_coinsEarned += coins;
        state.addCoins(coins);
        state.addXP(matchCount);

        ResourceManager::getInstance().playSound("match.wav", 90);
        dropAndRefill();
    }

    return found;
}

void MatchGame::dropAndRefill() {
    for (int c = 0; c < MATCH_GRID_COLS; ++c) {
        int emptyRow = MATCH_GRID_ROWS - 1;
        for (int r = MATCH_GRID_ROWS - 1; r >= 0; --r) {
            if (m_grid[r][c].colorId != -1) {
                if (emptyRow != r) {
                    m_grid[emptyRow][c].colorId = m_grid[r][c].colorId;
                    m_grid[r][c].colorId = -1;
                }
                emptyRow--;
            }
        }

        // Fill remaining empty cells at top
        for (int r = emptyRow; r >= 0; --r) {
            m_grid[r][c].colorId = rand() % 5;
        }
    }
}

void MatchGame::update(float dt, const InputState& input, PouState& state) {
    if (m_gameOver) {
        if (input.touchJustPressed || input.btnA || input.btnB) {
            m_wantsExit = true;
            ResourceManager::getInstance().stopMusic();
        }
        return;
    }

    // Countdown timer
    m_timeLeft -= dt;
    if (m_timeLeft <= 0.0f) {
        m_timeLeft = 0.0f;
        m_gameOver = true;
        ResourceManager::getInstance().playSound("game_over.wav", 110);
        return;
    }

    // Touch interaction
    if (input.touchJustPressed) {
        int col = static_cast<int>((input.touchX - m_gridStartX) / m_cellSize);
        int row = static_cast<int>((input.touchY - m_gridStartY) / m_cellSize);

        if (row >= 0 && row < MATCH_GRID_ROWS && col >= 0 && col < MATCH_GRID_COLS) {
            if (m_selectedR == -1) {
                // First selection
                m_selectedR = row;
                m_selectedC = col;
                ResourceManager::getInstance().playSound("click.wav", 50);
            } else {
                // Second selection: check if adjacent
                int dr = std::abs(row - m_selectedR);
                int dc = std::abs(col - m_selectedC);

                if ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)) {
                    // Swap
                    swapCells(m_selectedR, m_selectedC, row, col);

                    // Check if match created
                    if (!checkAndResolveMatches(state)) {
                        // Revert swap if no match
                        swapCells(m_selectedR, m_selectedC, row, col);
                        ResourceManager::getInstance().playSound("warning.wav", 50);
                    } else {
                        // Cascades check
                        while (checkAndResolveMatches(state)) {
                            // Keep clearing cascade matches
                        }
                    }
                    m_selectedR = -1;
                    m_selectedC = -1;
                } else {
                    // Pick new cell
                    m_selectedR = row;
                    m_selectedC = col;
                    ResourceManager::getInstance().playSound("click.wav", 50);
                }
            }
        } else {
            m_selectedR = -1;
            m_selectedC = -1;
        }
    }
}

void MatchGame::drawPouHead(SDL_Renderer* r, int cx, int cy, int size, int colorId) {
    SDL_Color col = MATCH_COLORS[std::clamp(colorId, 0, 5)];
    int rad = size / 2 - 2;

    // Body
    ResourceManager::fillCircle(r, cx, cy, rad, col);
    ResourceManager::drawCircle(r, cx, cy, rad, {50, 40, 30, 255}, 2);

    // Eyes
    int eyeDist = rad / 3;
    int eyeR = rad / 4;
    ResourceManager::fillCircle(r, cx - eyeDist, cy - 3, eyeR, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, cx + eyeDist, cy - 3, eyeR, {255, 255, 255, 255});
    ResourceManager::fillCircle(r, cx - eyeDist, cy - 3, eyeR / 2, {40, 40, 40, 255});
    ResourceManager::fillCircle(r, cx + eyeDist, cy - 3, eyeR / 2, {40, 40, 40, 255});

    // Mouth
    ResourceManager::fillCircle(r, cx, cy + eyeDist, eyeR / 2, {50, 30, 20, 255});
}

void MatchGame::renderGameElements(SDL_Renderer* r, float scale, bool isTV) {
    // Warm grid background
    SDL_SetRenderDrawColor(r, 245, 235, 220, 255);
    SDL_Rect bg = {0, 0, static_cast<int>(PouConfig::DRC_WIDTH * scale), static_cast<int>(PouConfig::DRC_HEIGHT * scale)};
    SDL_RenderFillRect(r, &bg);

    // Grid board backing
    int startX = static_cast<int>(m_gridStartX * scale);
    int startY = static_cast<int>(m_gridStartY * scale);
    int cSize = static_cast<int>(m_cellSize * scale);
    int gridW = MATCH_GRID_COLS * cSize;
    int gridH = MATCH_GRID_ROWS * cSize;

    SDL_Rect board = {startX - 8, startY - 8, gridW + 16, gridH + 16};
    ResourceManager::fillRoundedRect(r, board, static_cast<int>(14 * scale), {230, 218, 200, 255});
    SDL_SetRenderDrawColor(r, 190, 175, 150, 255);
    SDL_RenderDrawRect(r, &board);

    // Grid cells
    for (int row = 0; row < MATCH_GRID_ROWS; ++row) {
        for (int col = 0; col < MATCH_GRID_COLS; ++col) {
            int cx = startX + col * cSize + cSize / 2;
            int cy = startY + row * cSize + cSize / 2;

            // Highlight selected cell
            if (row == m_selectedR && col == m_selectedC) {
                SDL_Rect selRect = {startX + col * cSize + 2, startY + row * cSize + 2, cSize - 4, cSize - 4};
                ResourceManager::fillRoundedRect(r, selRect, 8, {255, 255, 180, 200});
            }

            int colorId = m_grid[row][col].colorId;
            if (colorId >= 0) {
                drawPouHead(r, cx, cy, cSize - 6, colorId);
            }
        }
    }

    // Top HUD
    std::string scoreText = "Score: " + std::to_string(m_score);
    ResourceManager::getInstance().renderText(r, scoreText, static_cast<int>(30 * scale), static_cast<int>(80 * scale), static_cast<int>(26 * scale), {80, 50, 30, 255});

    std::string coinsText = "Coins: " + std::to_string(m_coinsEarned);
    ResourceManager::getInstance().renderText(r, coinsText, static_cast<int>(30 * scale), static_cast<int>(130 * scale), static_cast<int>(24 * scale), {210, 150, 20, 255});

    // Timer Bar
    int timerW = static_cast<int>(150 * scale);
    int timerH = static_cast<int>(24 * scale);
    int tx = static_cast<int>(30 * scale);
    int ty = static_cast<int>(200 * scale);

    ResourceManager::getInstance().renderText(r, "Time Left", tx, ty - static_cast<int>(26 * scale), static_cast<int>(20 * scale), {90, 70, 50, 255});
    SDL_Rect tbg = {tx, ty, timerW, timerH};
    ResourceManager::fillRoundedRect(r, tbg, 6, {210, 195, 180, 255});

    float timeRatio = std::clamp(m_timeLeft / 60.0f, 0.0f, 1.0f);
    SDL_Color tColor = (timeRatio > 0.25f) ? SDL_Color{80, 200, 100, 255} : SDL_Color{220, 60, 60, 255};
    SDL_Rect tfill = {tx + 2, ty + 2, static_cast<int>((timerW - 4) * timeRatio), timerH - 4};
    ResourceManager::fillRoundedRect(r, tfill, 5, tColor);

    std::string secStr = std::to_string(static_cast<int>(m_timeLeft)) + "s";
    ResourceManager::getInstance().renderText(r, secStr, tx + timerW / 2, ty + 2, static_cast<int>(18 * scale), {255, 255, 255, 255}, true);

    // Game Over
    if (m_gameOver) {
        int cx = static_cast<int>(PouConfig::DRC_WIDTH * 0.5f * scale);
        int cy = static_cast<int>(PouConfig::DRC_HEIGHT * 0.5f * scale);

        SDL_Rect card = {cx - static_cast<int>(180 * scale), cy - static_cast<int>(110 * scale), static_cast<int>(360 * scale), static_cast<int>(220 * scale)};
        ResourceManager::fillRoundedRect(r, card, 16, {252, 246, 235, 245});
        SDL_SetRenderDrawColor(r, 200, 160, 120, 255);
        SDL_RenderDrawRect(r, &card);

        ResourceManager::getInstance().renderText(r, "TIME UP!", cx, cy - static_cast<int>(65 * scale), static_cast<int>(36 * scale), {180, 50, 40, 255}, true);
        std::string finalScore = "Final Score: " + std::to_string(m_score);
        ResourceManager::getInstance().renderText(r, finalScore, cx, cy - static_cast<int>(15 * scale), static_cast<int>(26 * scale), {80, 50, 30, 255}, true);
        ResourceManager::getInstance().renderText(r, "Tap screen or press A to return", cx, cy + static_cast<int>(45 * scale), static_cast<int>(18 * scale), {120, 90, 70, 255}, true);
    }
}

void MatchGame::renderTV(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 2.25f, true);
}

void MatchGame::renderDRC(SDL_Renderer* renderer, const PouState& state) {
    renderGameElements(renderer, 1.0f, false);
}
