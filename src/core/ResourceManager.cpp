// src/core/ResourceManager.cpp
#include "ResourceManager.hpp"
#include <iostream>
#include <fstream>
#include <cmath>

std::string ResourceManager::findAssetRoot() {
    // Check multiple candidate locations
    const std::vector<std::string> candidates = {
        "fs:/vol/content/assets/", // Wii U Title install path
        "content/assets/",          // Local relative to executable
        "assets/",                  // Extracted root
        "../content/assets/",       // Build subdir
        "../../content/assets/"     // Nested test
    };

    for (const auto& candidate : candidates) {
        std::ifstream test(candidate + "fonts/Cookies.ttf");
        if (test.good()) {
            return candidate;
        }
    }
    return "content/assets/";
}

void ResourceManager::init(SDL_Renderer* tvRenderer, SDL_Renderer* drcRenderer) {
    m_baseAssetPath = findAssetRoot();
    std::cout << "[ResourceManager] Base asset path: " << m_baseAssetPath << std::endl;
}

void ResourceManager::cleanup() {
    for (auto& pair : m_textures) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    m_textures.clear();

    for (auto& pair : m_fonts) {
        if (pair.second) {
            TTF_CloseFont(pair.second);
        }
    }
    m_fonts.clear();

    for (auto& pair : m_sounds) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_sounds.clear();

    for (auto& pair : m_musics) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_musics.clear();
}

std::string ResourceManager::resolvePath(const std::string& relativePath) const {
    if (relativePath.find(":/") != std::string::npos || relativePath[0] == '/') {
        return relativePath;
    }
    return m_baseAssetPath + relativePath;
}

SDL_Texture* ResourceManager::getTexture(SDL_Renderer* renderer, const std::string& path) {
    auto key = std::make_pair(renderer, path);
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        return it->second;
    }

    std::string fullPath = resolvePath(path);
    SDL_Surface* surface = IMG_Load(fullPath.c_str());
    if (!surface) {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        m_textures[key] = texture;
    }
    return texture;
}

void ResourceManager::getTextureDimensions(SDL_Renderer* renderer, const std::string& path, int& outW, int& outH) {
    SDL_Texture* tex = getTexture(renderer, path);
    if (tex) {
        SDL_QueryTexture(tex, nullptr, nullptr, &outW, &outH);
    } else {
        outW = 0;
        outH = 0;
    }
}

void ResourceManager::drawTexture(SDL_Renderer* renderer, const std::string& path, int x, int y, int w, int h, double angle) {
    SDL_Texture* tex = getTexture(renderer, path);
    if (!tex) return;

    SDL_Rect dstRect = {x, y, w, h};
    if (w <= 0 || h <= 0) {
        SDL_QueryTexture(tex, nullptr, nullptr, &dstRect.w, &dstRect.h);
    }

    if (angle == 0.0) {
        SDL_RenderCopy(renderer, tex, nullptr, &dstRect);
    } else {
        SDL_RenderCopyEx(renderer, tex, nullptr, &dstRect, angle, nullptr, SDL_FLIP_NONE);
    }
}

void ResourceManager::drawTextureCentered(SDL_Renderer* renderer, const std::string& path, int cx, int cy, int w, int h, double angle) {
    SDL_Texture* tex = getTexture(renderer, path);
    if (!tex) return;

    SDL_Rect dstRect;
    if (w <= 0 || h <= 0) {
        SDL_QueryTexture(tex, nullptr, nullptr, &dstRect.w, &dstRect.h);
    } else {
        dstRect.w = w;
        dstRect.h = h;
    }
    dstRect.x = cx - dstRect.w / 2;
    dstRect.y = cy - dstRect.h / 2;

    if (angle == 0.0) {
        SDL_RenderCopy(renderer, tex, nullptr, &dstRect);
    } else {
        SDL_RenderCopyEx(renderer, tex, nullptr, &dstRect, angle, nullptr, SDL_FLIP_NONE);
    }
}

TTF_Font* ResourceManager::getFont(int ptSize) {
    auto it = m_fonts.find(ptSize);
    if (it != m_fonts.end()) {
        return it->second;
    }

    std::string fontPath = resolvePath("fonts/Cookies.ttf");
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), ptSize);
    if (font) {
        m_fonts[ptSize] = font;
    }
    return font;
}

void ResourceManager::renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, int ptSize, SDL_Color color, bool center) {
    TTF_Font* font = getFont(ptSize);
    if (!font || text.empty()) return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dstRect;
        dstRect.w = surface->w;
        dstRect.h = surface->h;
        dstRect.x = center ? (x - surface->w / 2) : x;
        dstRect.y = center ? (y - surface->h / 2) : y;

        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void ResourceManager::getTextDimensions(const std::string& text, int ptSize, int& outW, int& outH) {
    TTF_Font* font = getFont(ptSize);
    if (font && !text.empty()) {
        TTF_SizeUTF8(font, text.c_str(), &outW, &outH);
    } else {
        outW = 0;
        outH = 0;
    }
}

void ResourceManager::playSound(const std::string& soundFile, int volume) {
    auto it = m_sounds.find(soundFile);
    Mix_Chunk* chunk = nullptr;
    if (it != m_sounds.end()) {
        chunk = it->second;
    } else {
        std::string fullPath = resolvePath("sounds/" + soundFile);
        chunk = Mix_LoadWAV(fullPath.c_str());
        if (chunk) {
            m_sounds[soundFile] = chunk;
        }
    }

    if (chunk) {
        Mix_VolumeChunk(chunk, volume);
        Mix_PlayChannel(-1, chunk, 0);
    }
}

void ResourceManager::playMusic(const std::string& musicFile, bool loop) {
    auto it = m_musics.find(musicFile);
    Mix_Music* music = nullptr;
    if (it != m_musics.end()) {
        music = it->second;
    } else {
        std::string fullPath = resolvePath("musics/" + musicFile);
        music = Mix_LoadMUS(fullPath.c_str());
        if (music) {
            m_musics[musicFile] = music;
        }
    }

    if (music) {
        Mix_PlayMusic(music, loop ? -1 : 0);
    }
}

void ResourceManager::stopMusic() {
    Mix_HaltMusic();
}

void ResourceManager::setMusicVolume(int volume) {
    Mix_VolumeMusic(volume);
}

void ResourceManager::fillCircle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void ResourceManager::drawCircle(SDL_Renderer* r, int cx, int cy, int radius, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    for (int t = 0; t < thickness; ++t) {
        int rad = radius - t;
        if (rad <= 0) break;
        int x = rad;
        int y = 0;
        int err = 0;

        while (x >= y) {
            SDL_RenderDrawPoint(r, cx + x, cy + y);
            SDL_RenderDrawPoint(r, cx + y, cy + x);
            SDL_RenderDrawPoint(r, cx - y, cy + x);
            SDL_RenderDrawPoint(r, cx - x, cy + y);
            SDL_RenderDrawPoint(r, cx - x, cy - y);
            SDL_RenderDrawPoint(r, cx - y, cy - x);
            SDL_RenderDrawPoint(r, cx + y, cy - x);
            SDL_RenderDrawPoint(r, cx + x, cy - y);

            if (err <= 0) {
                y += 1;
                err += 2 * y + 1;
            }
            if (err > 0) {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }
}

void ResourceManager::fillRoundedRect(SDL_Renderer* r, const SDL_Rect& rect, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    radius = std::min(radius, std::min(rect.w / 2, rect.h / 2));

    // Center body
    SDL_Rect inner = {rect.x + radius, rect.y, rect.w - 2 * radius, rect.h};
    SDL_RenderFillRect(r, &inner);

    // Left and right bodies
    SDL_Rect left = {rect.x, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_Rect right = {rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_RenderFillRect(r, &left);
    SDL_RenderFillRect(r, &right);

    // 4 Corner circles
    fillCircle(r, rect.x + radius, rect.y + radius, radius, color);
    fillCircle(r, rect.x + rect.w - radius - 1, rect.y + radius, radius, color);
    fillCircle(r, rect.x + radius, rect.y + rect.h - radius - 1, radius, color);
    fillCircle(r, rect.x + rect.w - radius - 1, rect.y + rect.h - radius - 1, radius, color);
}
