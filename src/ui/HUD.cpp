// src/ui/HUD.cpp
#include "HUD.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>

void HUD::renderGaugeCircleTV(SDL_Renderer* r, int cx, int cy, int radius, float value, const std::string& iconPath, const std::string& label, SDL_Color color) {
    // Background circle
    ResourceManager::fillCircle(r, cx, cy, radius, {255, 255, 255, 220});
    ResourceManager::drawCircle(r, cx, cy, radius, {180, 160, 130, 255}, 3);

    // Inner filled meter proportional to value
    int innerR = radius - 5;
    float ratio = std::clamp(value / 100.0f, 0.0f, 1.0f);
    int fillH = static_cast<int>(innerR * 2 * ratio);

    // Render fill mask
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, 160);
    int bottomY = cy + innerR;
    for (int y = bottomY - fillH; y <= bottomY; ++y) {
        int dy = y - cy;
        if (std::abs(dy) < innerR) {
            int dx = static_cast<int>(std::sqrt(innerR * innerR - dy * dy));
            SDL_RenderDrawLine(r, cx - dx, y, cx + dx, y);
        }
    }

    // Icon in center
    ResourceManager::getInstance().drawTextureCentered(r, iconPath, cx, cy - 6, 42, 42);

    // Percentage text
    std::string pctStr = std::to_string(static_cast<int>(value)) + "%";
    ResourceManager::getInstance().renderText(r, pctStr, cx, cy + 22, 18, {50, 40, 30, 255}, true);
}

void HUD::renderTV(SDL_Renderer* renderer, const PouState& state, const std::string& roomTitle) {
    // High-res Top Banner (1920x1080)
    SDL_Rect topBar = {0, 0, PouConfig::TV_WIDTH, 110};
    ResourceManager::fillRoundedRect(renderer, topBar, 0, {250, 242, 228, 240});
    SDL_SetRenderDrawColor(renderer, 210, 190, 160, 255);
    SDL_RenderDrawLine(renderer, 0, 110, PouConfig::TV_WIDTH, 110);

    // Left: Level & XP Bar
    std::string levelStr = "LEVEL " + std::to_string(state.stats.level);
    ResourceManager::getInstance().renderText(renderer, levelStr, 60, 32, 28, {90, 60, 35, 255});

    // XP Bar
    SDL_Rect xpBg = {210, 38, 220, 24};
    ResourceManager::fillRoundedRect(renderer, xpBg, 8, {220, 210, 195, 255});
    int reqXp = state.stats.level * 60;
    float xpRatio = std::clamp(static_cast<float>(state.stats.xp) / reqXp, 0.0f, 1.0f);
    SDL_Rect xpFill = {212, 40, static_cast<int>(216 * xpRatio), 20};
    ResourceManager::fillRoundedRect(renderer, xpFill, 6, {255, 185, 30, 255});

    // Center: Ambient Room Title
    ResourceManager::getInstance().renderText(renderer, roomTitle, PouConfig::TV_WIDTH / 2, 35, 42, {80, 50, 25, 255}, true);

    // Right: Coins
    ResourceManager::getInstance().drawTexture(renderer, "images/coin/1.png", 1620, 28, 48, 48);
    std::string coinStr = std::to_string(state.stats.coins) + " COINS";
    ResourceManager::getInstance().renderText(renderer, coinStr, 1680, 36, 28, {190, 140, 20, 255});

    // 4 Gauge Meters below top banner (Fullness, Health, Energy, Fun)
    int gaugeY = 165;
    int spacing = 160;
    int startX = (PouConfig::TV_WIDTH - (3 * spacing)) / 2;

    renderGaugeCircleTV(renderer, startX, gaugeY, 38, state.stats.fullness, "images/stats/fullness.png", "Fullness", {PouConfig::COLOR_BAR_FULLNESS.r, PouConfig::COLOR_BAR_FULLNESS.g, PouConfig::COLOR_BAR_FULLNESS.b, 255});
    renderGaugeCircleTV(renderer, startX + spacing, gaugeY, 38, state.stats.health, "images/stats/health.png", "Health", {PouConfig::COLOR_BAR_HEALTH.r, PouConfig::COLOR_BAR_HEALTH.g, PouConfig::COLOR_BAR_HEALTH.b, 255});
    renderGaugeCircleTV(renderer, startX + spacing * 2, gaugeY, 38, state.stats.energy, "images/stats/energy.png", "Energy", {PouConfig::COLOR_BAR_ENERGY.r, PouConfig::COLOR_BAR_ENERGY.g, PouConfig::COLOR_BAR_ENERGY.b, 255});
    renderGaugeCircleTV(renderer, startX + spacing * 3, gaugeY, 38, state.stats.fun, "images/stats/fun.png", "Fun", {PouConfig::COLOR_BAR_FUN.r, PouConfig::COLOR_BAR_FUN.g, PouConfig::COLOR_BAR_FUN.b, 255});
}

void HUD::renderDRC(SDL_Renderer* renderer, const PouState& state, const std::string& roomTitle) {
    // GamePad Top Navigation Bar (854x480)
    SDL_Rect topBar = {0, 0, PouConfig::DRC_WIDTH, 56};
    ResourceManager::fillRoundedRect(renderer, topBar, 0, {252, 246, 235, 235});
    SDL_SetRenderDrawColor(renderer, 215, 195, 170, 255);
    SDL_RenderDrawLine(renderer, 0, 56, PouConfig::DRC_WIDTH, 56);

    // Navigation Arrows: Previous & Next Room
    ResourceManager::getInstance().drawTexture(renderer, "images/icons/prev_room.png", 20, 10, 36, 36);
    ResourceManager::getInstance().drawTexture(renderer, "images/icons/next_room.png", PouConfig::DRC_WIDTH - 56, 10, 36, 36);

    // Room Title in center
    ResourceManager::getInstance().renderText(renderer, roomTitle, PouConfig::DRC_WIDTH / 2, 16, 26, {80, 50, 30, 255}, true);

    // Level indicator
    std::string lvlStr = "Lvl " + std::to_string(state.stats.level);
    ResourceManager::getInstance().renderText(renderer, lvlStr, 80, 18, 20, {110, 80, 50, 255});

    // Coins and Shop Button on right
    ResourceManager::getInstance().drawTexture(renderer, "images/coin/1.png", 640, 14, 28, 28);
    std::string coinStr = std::to_string(state.stats.coins);
    ResourceManager::getInstance().renderText(renderer, coinStr, 675, 18, 20, {180, 130, 20, 255});

    // Shop Button Icon
    ResourceManager::getInstance().drawTexture(renderer, "images/icons/settings_star.png", 735, 12, 32, 32);

    // Compact Stat Gauges (top status bar under navigation)
    int barW = 70;
    int barH = 12;
    int statY = 64;

    // Mini pill helper
    auto drawStatPill = [&](int x, float val, SDL_Color color, const std::string& iconPath) {
        ResourceManager::getInstance().drawTexture(renderer, iconPath, x - 18, statY - 4, 18, 18);
        SDL_Rect bg = {x + 4, statY, barW, barH};
        ResourceManager::fillRoundedRect(renderer, bg, 5, {220, 210, 195, 200});
        float ratio = std::clamp(val / 100.0f, 0.0f, 1.0f);
        if (ratio > 0.02f) {
            SDL_Rect fill = {x + 5, statY + 1, static_cast<int>((barW - 2) * ratio), barH - 2};
            ResourceManager::fillRoundedRect(renderer, fill, 4, color);
        }
    };

    drawStatPill(180, state.stats.fullness, {PouConfig::COLOR_BAR_FULLNESS.r, PouConfig::COLOR_BAR_FULLNESS.g, PouConfig::COLOR_BAR_FULLNESS.b, 255}, "images/stats/fullness.png");
    drawStatPill(330, state.stats.health, {PouConfig::COLOR_BAR_HEALTH.r, PouConfig::COLOR_BAR_HEALTH.g, PouConfig::COLOR_BAR_HEALTH.b, 255}, "images/stats/health.png");
    drawStatPill(480, state.stats.energy, {PouConfig::COLOR_BAR_ENERGY.r, PouConfig::COLOR_BAR_ENERGY.g, PouConfig::COLOR_BAR_ENERGY.b, 255}, "images/stats/energy.png");
    drawStatPill(630, state.stats.fun, {PouConfig::COLOR_BAR_FUN.r, PouConfig::COLOR_BAR_FUN.g, PouConfig::COLOR_BAR_FUN.b, 255}, "images/stats/fun.png");
}

bool HUD::handleTouch(int touchX, int touchY, bool justPressed, PouState& state, int& outRoomChange, bool& outOpenShop) {
    outRoomChange = 0;
    outOpenShop = false;

    if (!justPressed) return false;

    // Previous room arrow (top left 10 - 70, 0 - 56)
    if (touchX >= 10 && touchX <= 70 && touchY >= 0 && touchY <= 56) {
        outRoomChange = -1;
        ResourceManager::getInstance().playSound("click.wav", 64);
        return true;
    }

    // Next room arrow (top right 785 - 854, 0 - 56)
    if (touchX >= 785 && touchX <= 854 && touchY >= 0 && touchY <= 56) {
        outRoomChange = 1;
        ResourceManager::getInstance().playSound("click.wav", 64);
        return true;
    }

    // Shop button (725 - 775, 5 - 50)
    if (touchX >= 720 && touchX <= 780 && touchY >= 5 && touchY <= 52) {
        outOpenShop = true;
        return true;
    }

    return false;
}
