// src/core/PouState.cpp
#include "PouState.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

PouState::PouState() {
    initDefaultInventory();
}

void PouState::initDefaultInventory() {
    // Starting items in Fridge
    stats.foods["Apple.png"] = 3;
    stats.foods["Burger.png"] = 2;
    stats.foods["Cherries.png"] = 5;
    stats.foods["Banana Donut.png"] = 2;

    // Starting items in Lab Shelf
    stats.potions[static_cast<int>(PouConfig::PotionType::SMALL_HEALTH)] = 2;
    stats.potions[static_cast<int>(PouConfig::PotionType::ENERGIZER)] = 1;
}

void PouState::update(float dt) {
    if (stats.isAsleep) {
        stats.energy = std::min(100.0f, stats.energy + (ENERGY_REGEN * dt));
        // Slow fullness decay while sleeping
        stats.fullness = std::max(0.0f, stats.fullness - (FULLNESS_DECAY * 0.3f * dt));
    } else {
        stats.energy = std::max(0.0f, stats.energy - (ENERGY_DECAY * dt));
        stats.fullness = std::max(0.0f, stats.fullness - (FULLNESS_DECAY * dt));
        stats.fun = std::max(0.0f, stats.fun - (FUN_DECAY * dt));

        // Dirt accumulation when awake
        stats.dirtAccumulator += dt;
        if (stats.dirtAccumulator >= DIRT_INTERVAL) {
            stats.isDirty = true;
        }
    }

    // Health decay
    if (stats.isDirty) {
        stats.health = std::max(0.0f, stats.health - (HEALTH_DECAY_DIRTY * dt));
    }
    if (stats.fullness <= 0.0f) {
        stats.health = std::max(0.0f, stats.health - (HEALTH_DECAY_STARVE * dt));
    }
}

bool PouState::canFeed() const {
    return stats.fullness < 99.5f;
}

void PouState::feed(float nutritionPoints, const std::string& foodName) {
    stats.fullness = std::min(100.0f, stats.fullness + nutritionPoints);
    if (!foodName.empty() && stats.foods[foodName] > 0) {
        stats.foods[foodName]--;
        if (stats.foods[foodName] <= 0) {
            stats.foods.erase(foodName);
        }
    }
    // Overfeeding leads to obesity
    if (stats.fullness >= 98.0f) {
        stats.obesity = std::min(1.5f, stats.obesity + 0.05f);
    }
    addXP(8);
}

void PouState::rubSoap(float amount) {
    stats.soapAccumulator = std::min(100.0f, stats.soapAccumulator + amount);
}

void PouState::rinseShower(float amount) {
    if (stats.soapAccumulator > 0.0f) {
        stats.soapAccumulator = std::max(0.0f, stats.soapAccumulator - amount * 2.0f);
        if (stats.isDirty) {
            stats.dirtAccumulator = std::max(0.0f, stats.dirtAccumulator - amount * 5.0f);
            if (stats.dirtAccumulator <= 0.0f) {
                wash();
            }
        }
    }
}

void PouState::wash() {
    stats.isDirty = false;
    stats.dirtAccumulator = 0.0f;
    stats.soapAccumulator = 0.0f;
    addXP(15);
}

void PouState::addDirt(float amount) {
    stats.isDirty = true;
    stats.dirtAccumulator = std::min(DIRT_INTERVAL * 2.0f, stats.dirtAccumulator + amount);
}

void PouState::toggleSleep() {
    stats.isAsleep = !stats.isAsleep;
}

void PouState::setSleep(bool asleep) {
    stats.isAsleep = asleep;
}

void PouState::playPetting(float dt) {
    stats.fun = std::min(100.0f, stats.fun + (12.0f * dt));
    addXP(static_cast<int>(2.0f * dt));
}

void PouState::playWithBall(float funAmount) {
    stats.fun = std::min(100.0f, stats.fun + funAmount);
    addXP(5);
}

bool PouState::usePotion(PouConfig::PotionType type) {
    int key = static_cast<int>(type);
    if (stats.potions[key] <= 0) {
        return false;
    }
    stats.potions[key]--;
    if (stats.potions[key] <= 0) {
        stats.potions.erase(key);
    }

    switch (type) {
        case PouConfig::PotionType::HEALTH:
            stats.health = std::min(100.0f, stats.health + 50.0f);
            break;
        case PouConfig::PotionType::SMALL_HEALTH:
            stats.health = std::min(100.0f, stats.health + 25.0f);
            break;
        case PouConfig::PotionType::ENERGIZER:
            stats.energy = 100.0f;
            break;
        case PouConfig::PotionType::HUNGER:
            stats.fullness = 0.0f; // Empty stomach for more feeding
            break;
        case PouConfig::PotionType::FAT_BURNER:
            stats.obesity = 0.0f; // Return to standard slim size
            break;
        case PouConfig::PotionType::BABY:
            stats.ageStage = 0;
            break;
        case PouConfig::PotionType::ADULT:
            stats.ageStage = 2;
            break;
    }
    addXP(10);
    return true;
}

void PouState::addXP(int amount) {
    if (amount <= 0) return;
    stats.xp += amount;
    int requiredXp = stats.level * 60;
    while (stats.xp >= requiredXp) {
        stats.xp -= requiredXp;
        stats.level++;
        stats.coins += 50; // Level up reward
        requiredXp = stats.level * 60;
    }
}

void PouState::addCoins(int amount) {
    stats.coins += amount;
}

bool PouState::spendCoins(int amount) {
    if (stats.coins >= amount) {
        stats.coins -= amount;
        return true;
    }
    return false;
}

std::string PouState::getSavePath() const {
    // Try SD Card location first (for homebrew Wii U)
    std::ifstream testSD("sd:/wiiu/apps/Pou");
    if (testSD.good()) {
        return "sd:/wiiu/apps/Pou/pou_save.dat";
    }
    // Cafe OS save volume
    std::ifstream testSave("fs:/vol/save");
    if (testSave.good()) {
        return "fs:/vol/save/pou_save.dat";
    }
    // Local fallback
    return "pou_save.dat";
}

bool PouState::save(const std::string& filepath) {
    std::string path = filepath.empty() ? getSavePath() : filepath;
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << "POUSAVE_V1\n";
    file << stats.fullness << " " << stats.energy << " " << stats.fun << " " << stats.health << "\n";
    file << stats.coins << " " << stats.xp << " " << stats.level << "\n";
    file << (stats.isDirty ? 1 : 0) << " " << stats.dirtAccumulator << " " << stats.obesity << " " << stats.ageStage << "\n";
    file << stats.currentWallpaper << "\n";
    file << stats.currentBall << "\n";

    // Save foods
    file << stats.foods.size() << "\n";
    for (const auto& pair : stats.foods) {
        file << pair.first << " " << pair.second << "\n";
    }

    // Save potions
    file << stats.potions.size() << "\n";
    for (const auto& pair : stats.potions) {
        file << pair.first << " " << pair.second << "\n";
    }

    return true;
}

bool PouState::load(const std::string& filepath) {
    std::string path = filepath.empty() ? getSavePath() : filepath;
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string header;
    file >> header;
    if (header != "POUSAVE_V1") return false;

    file >> stats.fullness >> stats.energy >> stats.fun >> stats.health;
    file >> stats.coins >> stats.xp >> stats.level;
    int dirtyInt = 0;
    file >> dirtyInt >> stats.dirtAccumulator >> stats.obesity >> stats.ageStage;
    stats.isDirty = (dirtyInt != 0);

    file >> stats.currentWallpaper;
    file >> stats.currentBall;

    size_t foodCount = 0;
    if (file >> foodCount) {
        stats.foods.clear();
        for (size_t i = 0; i < foodCount; ++i) {
            std::string name;
            int count = 0;
            file >> name >> count;
            stats.foods[name] = count;
        }
    }

    size_t potionCount = 0;
    if (file >> potionCount) {
        stats.potions.clear();
        for (size_t i = 0; i < potionCount; ++i) {
            int pType = 0, count = 0;
            file >> pType >> count;
            stats.potions[pType] = count;
        }
    }

    return true;
}
