// src/entities/Pou.hpp
#pragma once

#include <SDL2/SDL.h>
#include "../core/Config.hpp"
#include "../core/PouState.hpp"
#include <vector>

enum class MouthExpression {
    NORMAL,
    HAPPY,
    VERY_HAPPY,
    OPEN_FOR_FOOD,
    EATING,
    FROWNING,
    SAYING_NO
};

struct HeartParticle {
    float x, y;
    float vy;
    float alpha;
    float size;
};

class Pou {
public:
    Pou();

    void update(float dt, PouState& state);
    void render(SDL_Renderer* renderer, float centerX, float centerY, float scale, bool isTV = false);

    // Expressions & State triggers
    void setTargetLook(float x, float y);
    void clearTargetLook();

    void openMouthForFood(bool open);
    void triggerEating();
    void triggerSayingNo();
    void triggerPetting();

    bool isPointInside(float px, float py, float centerX, float centerY, float scale) const;

    MouthExpression getMouthState() const { return m_mouthState; }

private:
    float m_breathingPhase = 0.0f;
    float m_breathingRadius = 0.0f;

    // Eye animation
    float m_blinkTimer = 0.0f;
    float m_nextBlinkInterval = 3.0f;
    bool m_isBlinking = false;
    float m_targetLookX = 0.0f;
    float m_targetLookY = 0.0f;
    bool m_hasTargetLook = false;
    float m_currentPupilOffsetX = 0.0f;
    float m_currentPupilOffsetY = 0.0f;

    // Mouth animation
    MouthExpression m_mouthState = MouthExpression::NORMAL;
    float m_eatingTimer = 0.0f;
    float m_sayingNoTimer = 0.0f;
    float m_pettingTimer = 0.0f;
    float m_headShakeOffset = 0.0f;

    // Floating heart particles for happy/petting
    std::vector<HeartParticle> m_hearts;

    void updateEyes(float dt, const PetStats& stats);
    void updateMouth(float dt);
    void updateParticles(float dt);

    void drawBody(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats);
    void drawEyes(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats);
    void drawMouth(SDL_Renderer* r, float cx, float cy, float scale);
    void drawDirt(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats);
    void drawSoapBubbles(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats);
    void drawHearts(SDL_Renderer* r);
};
