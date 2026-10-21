// src/core/Config.hpp
#pragma once

#include <cstdint>
#include <string>

namespace PouConfig {
    // Screen Resolutions
    constexpr int TV_WIDTH  = 1920;
    constexpr int TV_HEIGHT = 1080;

    constexpr int DRC_WIDTH  = 854;
    constexpr int DRC_HEIGHT = 480;

    // Target FPS
    constexpr int TARGET_FPS = 60;
    constexpr float FIXED_DELTA_TIME = 1.0f / 60.0f;

    // Room Identifiers
    enum class RoomId {
        KITCHEN = 0,
        BATHROOM,
        LABORATORY,
        GAME_ROOM,
        BEDROOM,
        HALL,
        COUNT
    };

    // Mini-Game Identifiers
    enum class GameId {
        NONE = 0,
        FOOD_DROP,
        SKY_FALL,
        POU_JUMP,
        POU_MATCH
    };

    // Potion Types
    enum class PotionType {
        HEALTH,
        SMALL_HEALTH,
        ENERGIZER,
        HUNGER,
        FAT_BURNER,
        BABY,
        ADULT
    };

    // Color definitions (RGBA)
    struct ColorRGBA {
        uint8_t r, g, b, a;
    };

    constexpr ColorRGBA COLOR_POU_DEFAULT   = {199, 149, 90, 255}; // Classic Brown
    constexpr ColorRGBA COLOR_POU_OUTLINE   = {120, 80, 40, 255};
    constexpr ColorRGBA COLOR_EYE_WHITE     = {255, 255, 255, 255};
    constexpr ColorRGBA COLOR_EYE_PUPIL     = {40, 40, 40, 255};
    constexpr ColorRGBA COLOR_MOUTH         = {50, 30, 20, 255};

    constexpr ColorRGBA COLOR_TV_BG         = {245, 230, 200, 255};
    constexpr ColorRGBA COLOR_DRC_BG        = {235, 220, 190, 255};

    // Stats Bar Colors
    constexpr ColorRGBA COLOR_BAR_FULLNESS  = {255, 120, 50, 255}; // Orange
    constexpr ColorRGBA COLOR_BAR_HEALTH    = {230, 50, 50, 255};   // Red
    constexpr ColorRGBA COLOR_BAR_ENERGY    = {60, 180, 240, 255};  // Cyan
    constexpr ColorRGBA COLOR_BAR_FUN       = {100, 215, 60, 255};  // Green
    constexpr ColorRGBA COLOR_BAR_COINS     = {255, 215, 0, 255};   // Gold
}
