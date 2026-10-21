// src/core/ResourceManager.hpp
#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>
#include <vector>

class ResourceManager {
public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    void init(SDL_Renderer* tvRenderer, SDL_Renderer* drcRenderer);
    void cleanup();

    // Asset path resolver
    std::string resolvePath(const std::string& relativePath) const;

    // Texture handling
    SDL_Texture* getTexture(SDL_Renderer* renderer, const std::string& path);
    void getTextureDimensions(SDL_Renderer* renderer, const std::string& path, int& outW, int& outH);

    void drawTexture(SDL_Renderer* renderer, const std::string& path, int x, int y, int w = -1, int h = -1, double angle = 0.0);
    void drawTextureCentered(SDL_Renderer* renderer, const std::string& path, int cx, int cy, int w = -1, int h = -1, double angle = 0.0);

    // Font and text rendering
    TTF_Font* getFont(int ptSize);
    void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int ptSize, SDL_Color color, bool center = false);
    void getTextDimensions(const std::string& text, int ptSize, int& outW, int& outH);

    // Audio handling
    void playSound(const std::string& soundFile, int volume = 128);
    void playMusic(const std::string& musicFile, bool loop = true);
    void stopMusic();
    void setMusicVolume(int volume);

    // Graphical shape primitives
    static void fillCircle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color);
    static void drawCircle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color, int thickness = 1);
    static void fillRoundedRect(SDL_Renderer* r, const SDL_Rect& rect, int radius, SDL_Color color);

private:
    ResourceManager() = default;
    ~ResourceManager() { cleanup(); }

    std::map<std::pair<SDL_Renderer*, std::string>, SDL_Texture*> m_textures;
    std::map<int, TTF_Font*> m_fonts;
    std::map<std::string, Mix_Chunk*> m_sounds;
    std::map<std::string, Mix_Music*> m_musics;

    std::string m_baseAssetPath;
    std::string findAssetRoot();
};
