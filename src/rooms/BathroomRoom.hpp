// src/rooms/BathroomRoom.hpp
#pragma once

#include "Room.hpp"

class BathroomRoom : public Room {
public:
    BathroomRoom();

    void enter(PouState& state, Pou& pou) override;
    void update(float dt, PouState& state, Pou& pou) override;
    void renderTV(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void renderDRC(SDL_Renderer* renderer, PouState& state, Pou& pou) override;
    void handleTouch(int touchX, int touchY, bool justPressed, bool isTouched, bool justReleased, PouState& state, Pou& pou) override;

    std::string getRoomName() const override { return "Bathroom"; }

private:
    bool m_isDraggingSoap = false;
    float m_soapX = 260.0f;
    float m_soapY = 415.0f;
    const float m_soapTrayX = 260.0f;
    const float m_soapTrayY = 415.0f;

    bool m_isDraggingShower = false;
    float m_showerX = 590.0f;
    float m_showerY = 415.0f;
    const float m_showerTrayX = 590.0f;
    const float m_showerTrayY = 415.0f;

    void drawWallpaper(SDL_Renderer* r, int w, int h, const std::string& wp);
};
