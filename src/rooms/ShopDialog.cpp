// src/rooms/ShopDialog.cpp
#include "ShopDialog.hpp"
#include "../core/ResourceManager.hpp"

ShopDialog::ShopDialog() {
    initCatalog();
}

void ShopDialog::initCatalog() {
    // Food category
    m_items.push_back({"Apple.png", "Apple", "food", "images/food/Apple.png", 15, -1});
    m_items.push_back({"Burger.png", "Burger", "food", "images/food/Burger.png", 30, -1});
    m_items.push_back({"Cherries.png", "Cherries", "food", "images/food/Cherries.png", 12, -1});
    m_items.push_back({"Banana Donut.png", "Donut", "food", "images/food/Banana Donut.png", 20, -1});
    m_items.push_back({"Chocolate Cake.png", "Cake", "food", "images/food/Chocolate Cake.png", 35, -1});

    // Potions category
    m_items.push_back({"Small Health", "Small Health", "potion", "images/potions/Small Health.png", 25, static_cast<int>(PouConfig::PotionType::SMALL_HEALTH)});
    m_items.push_back({"Health Potion", "Health Potion", "potion", "images/potions/Health Potion.png", 50, static_cast<int>(PouConfig::PotionType::HEALTH)});
    m_items.push_back({"Energizer", "Energizer", "potion", "images/potions/Energizer.png", 40, static_cast<int>(PouConfig::PotionType::ENERGIZER)});
    m_items.push_back({"Hunger Potion", "Hunger Potion", "potion", "images/potions/Hunger Potion.png", 30, static_cast<int>(PouConfig::PotionType::HUNGER)});
    m_items.push_back({"Fat Burner", "Fat Burner", "potion", "images/potions/Fat Burner.png", 35, static_cast<int>(PouConfig::PotionType::FAT_BURNER)});

    // Wallpapers category
    m_items.push_back({"112.png", "Warm Stripe", "wallpaper", "images/wallpapers/112.png", 80, -1});
    m_items.push_back({"121.png", "Dots", "wallpaper", "images/wallpapers/121.png", 80, -1});
    m_items.push_back({"131.png", "Squares", "wallpaper", "images/wallpapers/131.png", 100, -1});
    m_items.push_back({"141.png", "Flowers", "wallpaper", "images/wallpapers/141.png", 100, -1});
}

void ShopDialog::open() {
    m_isOpen = true;
    m_currentCategory = 0;
    ResourceManager::getInstance().playSound("click.wav", 80);
}

void ShopDialog::close() {
    m_isOpen = false;
    ResourceManager::getInstance().playSound("click.wav", 80);
}

void ShopDialog::render(SDL_Renderer* renderer, const PouState& state) {
    if (!m_isOpen) return;

    // Dark backdrop
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT};
    SDL_RenderFillRect(renderer, &full);

    // Modal Card
    SDL_Rect card = {80, 40, 694, 400};
    ResourceManager::fillRoundedRect(renderer, card, 20, {253, 248, 238, 255});
    SDL_SetRenderDrawColor(renderer, 190, 160, 120, 255);
    SDL_RenderDrawRect(renderer, &card);

    // Header: Shop Title, Coins, Close button
    ResourceManager::getInstance().renderText(renderer, "Pou Shop", 110, 55, 30, {80, 50, 30, 255});
    std::string coinsStr = std::to_string(state.stats.coins) + " Coins";
    ResourceManager::getInstance().renderText(renderer, coinsStr, 600, 60, 22, {210, 160, 20, 255}, true);
    ResourceManager::getInstance().renderText(renderer, "[X]", 740, 55, 24, {180, 60, 60, 255}, true);

    // Category Tabs: 0: Food, 1: Potions, 2: Wallpapers
    const char* tabs[] = {"Foods", "Potions", "Wallpapers"};
    for (int t = 0; t < 3; ++t) {
        SDL_Rect tabRect = {110 + t * 140, 95, 125, 36};
        SDL_Color tabBg = (m_currentCategory == t) ? SDL_Color{220, 190, 150, 255} : SDL_Color{240, 230, 215, 255};
        ResourceManager::fillRoundedRect(renderer, tabRect, 10, tabBg);
        ResourceManager::getInstance().renderText(renderer, tabs[t], tabRect.x + 62, tabRect.y + 7, 18, {70, 50, 30, 255}, true);
    }

    // List filtered items
    std::string catName = (m_currentCategory == 0) ? "food" : (m_currentCategory == 1 ? "potion" : "wallpaper");
    std::vector<const ShopItemDef*> filtered;
    for (const auto& item : m_items) {
        if (item.category == catName) {
            filtered.push_back(&item);
        }
    }

    for (size_t i = 0; i < filtered.size() && i < 4; ++i) {
        const auto* it = filtered[i];
        int rowY = 145 + static_cast<int>(i) * 68;

        SDL_Rect itemBox = {105, rowY, 644, 60};
        ResourceManager::fillRoundedRect(renderer, itemBox, 12, {245, 238, 225, 255});
        SDL_SetRenderDrawColor(renderer, 215, 200, 175, 255);
        SDL_RenderDrawRect(renderer, &itemBox);

        // Icon
        ResourceManager::getInstance().drawTextureCentered(renderer, it->iconPath, 140, rowY + 30, 48, 48);

        // Name
        ResourceManager::getInstance().renderText(renderer, it->name, 180, rowY + 16, 22, {70, 45, 25, 255});

        // Price
        std::string priceStr = std::to_string(it->price) + " Coins";
        ResourceManager::getInstance().renderText(renderer, priceStr, 500, rowY + 18, 20, {180, 140, 20, 255});

        // Buy button
        SDL_Rect buyBtn = {620, rowY + 12, 110, 36};
        bool canAfford = (state.stats.coins >= it->price);
        SDL_Color btnColor = canAfford ? SDL_Color{100, 200, 80, 255} : SDL_Color{180, 180, 180, 255};
        ResourceManager::fillRoundedRect(renderer, buyBtn, 10, btnColor);
        ResourceManager::getInstance().renderText(renderer, "BUY", buyBtn.x + 55, buyBtn.y + 6, 18, {255, 255, 255, 255}, true);
    }
}

void ShopDialog::handleTouch(int touchX, int touchY, bool justPressed, PouState& state) {
    if (!m_isOpen || !justPressed) return;

    // Close button
    if (touchX >= 710 && touchX <= 765 && touchY >= 45 && touchY <= 85) {
        close();
        return;
    }

    // Category Tabs (110 + t * 140, 95, 125, 36)
    for (int t = 0; t < 3; ++t) {
        int tx = 110 + t * 140;
        if (touchX >= tx && touchX <= tx + 125 && touchY >= 95 && touchY <= 135) {
            m_currentCategory = t;
            ResourceManager::getInstance().playSound("click.wav", 64);
            return;
        }
    }

    // Buy buttons
    std::string catName = (m_currentCategory == 0) ? "food" : (m_currentCategory == 1 ? "potion" : "wallpaper");
    std::vector<ShopItemDef*> filtered;
    for (auto& item : m_items) {
        if (item.category == catName) {
            filtered.push_back(&item);
        }
    }

    for (size_t i = 0; i < filtered.size() && i < 4; ++i) {
        auto* it = filtered[i];
        int rowY = 145 + static_cast<int>(i) * 68;
        int btnX = 620;
        int btnY = rowY + 12;

        if (touchX >= btnX && touchX <= btnX + 110 && touchY >= btnY && touchY <= btnY + 36) {
            if (state.spendCoins(it->price)) {
                ResourceManager::getInstance().playSound("buy.wav", 100);
                if (it->category == "food") {
                    state.stats.foods[it->id]++;
                } else if (it->category == "potion") {
                    state.stats.potions[it->potionType]++;
                } else if (it->category == "wallpaper") {
                    state.stats.currentWallpaper = it->id;
                }
            } else {
                ResourceManager::getInstance().playSound("warning.wav", 80);
            }
            return;
        }
    }
}
