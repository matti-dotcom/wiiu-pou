// src/core/PouState.hpp
#pragma once

#include "Config.hpp"
#include <string>
#include <map>
#include <algorithm>

struct PetStats {
    float fullness = 80.0f;  // Decreases over time; replenished by eating
    float energy   = 85.0f;  // Decreases when awake; replenished by sleeping
    float fun      = 75.0f;  // Decreases over time; replenished by games & ball
    float health   = 100.0f; // Decreases if dirty or starving; replenished by potions

    int coins      = 250;
    int xp         = 0;
    int level      = 1;

    bool isDirty   = false;
    bool isAsleep  = false;
    float dirtAccumulator = 0.0f;
    float soapAccumulator = 0.0f; // Bubbles lathered by soap
    float obesity         = 0.0f; // Size multiplier from overeating
    int ageStage          = 1;    // 0: Baby, 1: Kid, 2: Adult

    // Current customizations
    int currentRoomIndex  = 0;
    std::string currentWallpaper = "111.png";
    std::string currentBall      = "Basketball.png";
    std::string currentHat       = "";
    PouConfig::ColorRGBA bodyColor = PouConfig::COLOR_POU_DEFAULT;

    // Inventory
    std::map<std::string, int> foods;
    std::map<int, int> potions; // PotionType -> count
};

class PouState {
public:
    PetStats stats;

    // Real-time decay rates per second (derived from Pou.jar constants)
    const float FULLNESS_DECAY      = 0.020f;
    const float ENERGY_DECAY        = 0.015f;
    const float ENERGY_REGEN        = 0.150f; // Regenerates when asleep
    const float FUN_DECAY           = 0.025f;
    const float HEALTH_DECAY_DIRTY  = 0.035f;
    const float HEALTH_DECAY_STARVE = 0.050f;
    const float DIRT_INTERVAL       = 180.0f; // Seconds of activity until Pou gets dirty

    PouState();

    void update(float dt);

    // Gameplay interactions
    bool canFeed() const;
    void feed(float nutritionPoints, const std::string& foodName = "");
    void rubSoap(float amount = 1.0f);
    void rinseShower(float amount = 1.0f);
    void wash();
    void addDirt(float amount = 1.0f);

    void toggleSleep();
    void setSleep(bool asleep);

    void playPetting(float dt);
    void playWithBall(float funAmount = 5.0f);
    bool usePotion(PouConfig::PotionType type);

    // Economy & Progression
    void addXP(int amount);
    void addCoins(int amount);
    bool spendCoins(int amount);

    // Save & Load
    bool save(const std::string& filepath = "");
    bool load(const std::string& filepath = "");

private:
    void initDefaultInventory();
    std::string getSavePath() const;
};
