// src/main.cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#ifdef __WIIU__
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <vpad/input.h>
#endif

#include "core/Config.hpp"
#include "core/PouState.hpp"
#include "core/ResourceManager.hpp"
#include "core/InputManager.hpp"
#include "entities/Pou.hpp"
#include "ui/HUD.hpp"

#include "rooms/Room.hpp"
#include "rooms/KitchenRoom.hpp"
#include "rooms/BathroomRoom.hpp"
#include "rooms/LaboratoryRoom.hpp"
#include "rooms/BedroomRoom.hpp"
#include "rooms/GameRoom.hpp"
#include "rooms/ShopDialog.hpp"

#include "games/MiniGame.hpp"
#include "games/FoodGame.hpp"
#include "games/FallGame.hpp"
#include "games/JumpGame.hpp"
#include "games/MatchGame.hpp"

#include <vector>
#include <memory>
#include <iostream>

int main(int argc, char** argv) {
#ifdef __WIIU__
    WHBProcInit();
#endif

    // Initialize SDL2 subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << std::endl;
    }
    Mix_AllocateChannels(16);

    // Create Dual-Screen Windows and Renderers
    // TV Window (1920x1080)
    SDL_Window* tvWindow = SDL_CreateWindow(
        "Pou TV",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        PouConfig::TV_WIDTH, PouConfig::TV_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* tvRenderer = SDL_CreateRenderer(tvWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // DRC Window (854x480)
    SDL_Window* drcWindow = SDL_CreateWindow(
        "Pou GamePad",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        PouConfig::DRC_WIDTH, PouConfig::DRC_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* drcRenderer = SDL_CreateRenderer(drcWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Initialize Subsystems
    ResourceManager::getInstance().init(tvRenderer, drcRenderer);
    InputManager::getInstance().init();

    // Game Core State & Entities
    PouState state;
    state.load(); // Load saved game if exists

    Pou pou;
    HUD hud;
    ShopDialog shop;

    // Rooms Setup
    std::vector<std::unique_ptr<Room>> rooms;
    rooms.push_back(std::make_unique<KitchenRoom>());
    rooms.push_back(std::make_unique<BathroomRoom>());
    rooms.push_back(std::make_unique<LaboratoryRoom>());
    rooms.push_back(std::make_unique<GameRoom>());
    rooms.push_back(std::make_unique<BedroomRoom>());

    int currentRoomIdx = std::clamp(state.stats.currentRoomIndex, 0, static_cast<int>(rooms.size()) - 1);
    rooms[currentRoomIdx]->enter(state, pou);

    // Mini-Games Setup
    std::unique_ptr<MiniGame> activeGame = nullptr;

    bool running = true;
    uint32_t lastTicks = SDL_GetTicks();
    float autoSaveTimer = 0.0f;

    // Main Game Loop
    while (running
#ifdef __WIIU__
           && WHBProcIsRunning()
#endif
    ) {
        // Delta time
        uint32_t currentTicks = SDL_GetTicks();
        float dt = (currentTicks - lastTicks) / 1000.0f;
        if (dt > 0.1f) dt = 0.1f; // Clamp delta time to avoid large jumps
        lastTicks = currentTicks;

        // Auto-save every 30 seconds
        autoSaveTimer += dt;
        if (autoSaveTimer >= 30.0f) {
            autoSaveTimer = 0.0f;
            state.stats.currentRoomIndex = currentRoomIdx;
            state.save();
        }

        // SDL Event pump
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Input update
        InputManager::getInstance().update(drcWindow);
        const InputState& input = InputManager::getInstance().getState();

        if (input.homePressed) {
            running = false;
        }

        // ==========================================
        // UPDATE LOGIC
        // ==========================================
        if (activeGame) {
            // Mini-Game is active
            activeGame->update(dt, input, state);

            if (activeGame->wantsExit()) {
                activeGame = nullptr;
                rooms[currentRoomIdx]->enter(state, pou);
            }
        } else {
            // Normal Room & Virtual Pet mode
            state.update(dt);
            pou.update(dt, state);

            Room* curRoom = rooms[currentRoomIdx].get();
            curRoom->update(dt, state, pou);

            // Handle touch input
            if (shop.isOpen()) {
                shop.handleTouch(input.touchX, input.touchY, input.touchJustPressed, state);
            } else {
                int roomChange = 0;
                bool openShop = false;

                // Check HUD top bar navigation
                bool hudHandled = hud.handleTouch(input.touchX, input.touchY, input.touchJustPressed, state, roomChange, openShop);

                if (openShop) {
                    shop.open();
                } else if (roomChange != 0) {
                    curRoom->exit(state, pou);
                    currentRoomIdx = (currentRoomIdx + roomChange + static_cast<int>(rooms.size())) % rooms.size();
                    state.stats.currentRoomIndex = currentRoomIdx;
                    rooms[currentRoomIdx]->enter(state, pou);
                } else if (!hudHandled) {
                    // Pass touch to active room
                    curRoom->handleTouch(input.touchX, input.touchY, input.touchJustPressed, input.isTouched, input.touchJustReleased, state, pou);
                }
            }

            // Check if GameRoom requested launching a mini-game
            auto* gameRoom = dynamic_cast<GameRoom*>(curRoom);
            if (gameRoom && gameRoom->getRequestedGame() != PouConfig::GameId::NONE) {
                PouConfig::GameId requested = gameRoom->getRequestedGame();
                gameRoom->clearRequestedGame();

                switch (requested) {
                    case PouConfig::GameId::FOOD_DROP:
                        activeGame = std::make_unique<FoodGame>();
                        break;
                    case PouConfig::GameId::SKY_FALL:
                        activeGame = std::make_unique<FallGame>();
                        break;
                    case PouConfig::GameId::POU_JUMP:
                        activeGame = std::make_unique<JumpGame>();
                        break;
                    case PouConfig::GameId::POU_MATCH:
                        activeGame = std::make_unique<MatchGame>();
                        break;
                    default:
                        break;
                }

                if (activeGame) {
                    activeGame->start(state);
                }
            }
        }

        // ==========================================
        // RENDER TV SCENE (1920x1080)
        // ==========================================
        SDL_SetRenderDrawColor(tvRenderer, PouConfig::COLOR_TV_BG.r, PouConfig::COLOR_TV_BG.g, PouConfig::COLOR_TV_BG.b, 255);
        SDL_RenderClear(tvRenderer);

        if (activeGame) {
            activeGame->renderTV(tvRenderer, state);
        } else {
            rooms[currentRoomIdx]->renderTV(tvRenderer, state, pou);
            hud.renderTV(tvRenderer, state, rooms[currentRoomIdx]->getRoomName());
        }
        SDL_RenderPresent(tvRenderer);

        // ==========================================
        // RENDER GAMEPAD DRC SCENE (854x480)
        // ==========================================
        SDL_SetRenderDrawColor(drcRenderer, PouConfig::COLOR_DRC_BG.r, PouConfig::COLOR_DRC_BG.g, PouConfig::COLOR_DRC_BG.b, 255);
        SDL_RenderClear(drcRenderer);

        if (activeGame) {
            activeGame->renderDRC(drcRenderer, state);
        } else {
            rooms[currentRoomIdx]->renderDRC(drcRenderer, state, pou);
            hud.renderDRC(drcRenderer, state, rooms[currentRoomIdx]->getRoomName());

            if (shop.isOpen()) {
                shop.render(drcRenderer, state);
            }
        }
        SDL_RenderPresent(drcRenderer);
    }

    // Save before exiting
    state.stats.currentRoomIndex = currentRoomIdx;
    state.save();

    // Clean up
    ResourceManager::getInstance().cleanup();

    if (drcRenderer) SDL_DestroyRenderer(drcRenderer);
    if (drcWindow) SDL_DestroyWindow(drcWindow);
    if (tvRenderer) SDL_DestroyRenderer(tvRenderer);
    if (tvWindow) SDL_DestroyWindow(tvWindow);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

#ifdef __WIIU__
    WHBProcShutdown();
#endif

    return 0;
}
