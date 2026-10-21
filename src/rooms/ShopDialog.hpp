// src/rooms/ShopDialog.hpp
#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include "../core/PouState.hpp"

struct ShopItemDef {
    std::string id;
    std::string name;
    std::string category; // "food", "potion", "wallpaper"
    std::string iconPath;
    int price;
    int potionType; // If potion
};

class ShopDialog {
public:
    ShopDialog();

    void open();
    void close();
    bool isOpen() const { return m_isOpen; }

    void render(SDL_Renderer* renderer, const PouState& state);
    void handleTouch(int touchX, int touchY, bool justPressed, PouState& state);

private:
    bool m_isOpen = false;
    int m_currentCategory = 0; // 0: Food, 1: Potions, 2: Wallpapers
    std::vector<ShopItemDef> m_items;

    void initCatalog();
};
