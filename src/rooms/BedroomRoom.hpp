// src/rooms/BedroomRoom.hpp
#pragma once

#include "Room.hpp"
#include <vector>

struct SleepZzz {
    float x, y;
    float vy;
    float alpha;
    float size;
};

class BedroomRoom : public Room {
public:
    BedroomRoom();

    void enter(PouState& state, Pou& pou) override;
    void update(float dt, PouState& state, Pou& pou) override;
    void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) override;

    std::string getRoomName() const override { return "Bedroom"; }

private:
    float m_lampX = 640.0f;
    float m_lampY = 240.0f;
    float m_closetX = 160.0f;
    float m_closetY = 260.0f;

    std::vector<SleepZzz> m_zzzs;
    float m_zzzTimer = 0.0f;

    void drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp);
    void drawZzzs(SDL_Renderer* r, float scale = 1.0f);
};
