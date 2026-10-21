// src/entities/Pou.cpp
#include "Pou.hpp"
#include "../core/ResourceManager.hpp"
#include <cmath>
#include <cstdlib>

Pou::Pou() {
    m_breathingPhase = 0.0f;
    m_nextBlinkInterval = 2.5f + (rand() % 2000) / 1000.0f;
}

void Pou::update(float dt, PouState& state) {
    // Breathing animation
    float breathingSpeed = state.stats.isAsleep ? 1.5f : 3.0f;
    m_breathingPhase += dt * breathingSpeed;
    m_breathingRadius = std::sin(m_breathingPhase) * 2.5f;

    updateEyes(dt, state.stats);
    updateMouth(dt);
    updateParticles(dt);
}

void Pou::updateEyes(float dt, const PetStats& stats) {
    if (stats.isAsleep) {
        m_isBlinking = true;
        return;
    }

    m_blinkTimer += dt;
    if (m_blinkTimer >= m_nextBlinkInterval) {
        m_isBlinking = true;
        if (m_blinkTimer >= m_nextBlinkInterval + 0.16f) { // Blink duration 0.16s
            m_isBlinking = false;
            m_blinkTimer = 0.0f;
            m_nextBlinkInterval = 2.5f + (rand() % 3000) / 1000.0f;
        }
    } else {
        m_isBlinking = false;
    }

    // Smooth pupil look direction
    float desiredOffsetX = 0.0f;
    float desiredOffsetY = 0.0f;

    if (m_hasTargetLook) {
        desiredOffsetX = std::clamp(m_targetLookX, -10.0f, 10.0f);
        desiredOffsetY = std::clamp(m_targetLookY, -8.0f, 8.0f);
    }

    m_currentPupilOffsetX += (desiredOffsetX - m_currentPupilOffsetX) * (dt * 8.0f);
    m_currentPupilOffsetY += (desiredOffsetY - m_currentPupilOffsetY) * (dt * 8.0f);
}

void Pou::updateMouth(float dt) {
    if (m_eatingTimer > 0.0f) {
        m_eatingTimer -= dt;
        m_mouthState = MouthExpression::EATING;
    } else if (m_sayingNoTimer > 0.0f) {
        m_sayingNoTimer -= dt;
        m_headShakeOffset = std::sin(m_sayingNoTimer * 30.0f) * 12.0f;
        m_mouthState = MouthExpression::SAYING_NO;
    } else if (m_pettingTimer > 0.0f) {
        m_pettingTimer -= dt;
        m_headShakeOffset = 0.0f;
        m_mouthState = MouthExpression::VERY_HAPPY;
    } else {
        m_headShakeOffset = 0.0f;
        if (m_mouthState != MouthExpression::OPEN_FOR_FOOD) {
            m_mouthState = MouthExpression::NORMAL;
        }
    }
}

void Pou::updateParticles(float dt) {
    for (auto it = m_hearts.begin(); it != m_hearts.end();) {
        it->y -= it->vy * dt;
        it->alpha -= dt * 0.8f;
        if (it->alpha <= 0.0f) {
            it = m_hearts.erase(it);
        } else {
            ++it;
        }
    }
}

void Pou::setTargetLook(float x, float y) {
    m_targetLookX = x;
    m_targetLookY = y;
    m_hasTargetLook = true;
}

void Pou::clearTargetLook() {
    m_hasTargetLook = false;
}

void Pou::openMouthForFood(bool open) {
    if (m_eatingTimer <= 0.0f && m_sayingNoTimer <= 0.0f) {
        m_mouthState = open ? MouthExpression::OPEN_FOR_FOOD : MouthExpression::NORMAL;
    }
}

void Pou::triggerEating() {
    m_eatingTimer = 1.2f;
    ResourceManager::getInstance().playSound("a7a7e7e7e7e.wav");
}

void Pou::triggerSayingNo() {
    m_sayingNoTimer = 0.8f;
    ResourceManager::getInstance().playSound("no.wav");
}

void Pou::triggerPetting() {
    m_pettingTimer = 0.6f;
    // Spawn heart particle
    if (m_hearts.size() < 12) {
        HeartParticle h;
        h.x = (rand() % 80) - 40.0f;
        h.y = -60.0f;
        h.vy = 40.0f + (rand() % 40);
        h.alpha = 1.0f;
        h.size = 12.0f + (rand() % 10);
        m_hearts.push_back(h);
    }
    ResourceManager::getInstance().playSound("naa22.wav", 40);
}

bool Pou::isPointInside(float px, float py, float centerX, float centerY, float scale) const {
    float dx = (px - centerX) / (scale * 75.0f);
    float dy = (py - centerY) / (scale * 65.0f);
    return (dx * dx + dy * dy) <= 1.0f;
}

void Pou::render(SDL_Renderer* renderer, float centerX, float centerY, float scale, bool isTV) {
    float cx = centerX + m_headShakeOffset;
    float cy = centerY + (m_breathingRadius * scale * 0.3f);
    float currentScale = scale + (m_breathingRadius * 0.015f);

    PetStats dummyStats;

    drawBody(renderer, cx, cy, currentScale, dummyStats);
    drawDirt(renderer, cx, cy, currentScale, dummyStats);
    drawSoapBubbles(renderer, cx, cy, currentScale, dummyStats);
    drawEyes(renderer, cx, cy, currentScale, dummyStats);
    drawMouth(renderer, cx, cy, currentScale);
    drawHearts(renderer);
}

void Pou::drawBody(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats) {
    // Teardrop Pou Body Shape
    // We compose the smooth body with overlapping anti-aliased discs and trapezoidal body
    int baseW = static_cast<int>(150 * scale);
    int baseH = static_cast<int>(130 * scale);

    SDL_Color bodyColor = {PouConfig::COLOR_POU_DEFAULT.r, PouConfig::COLOR_POU_DEFAULT.g, PouConfig::COLOR_POU_DEFAULT.b, 255};
    SDL_Color outlineColor = {PouConfig::COLOR_POU_OUTLINE.r, PouConfig::COLOR_POU_OUTLINE.g, PouConfig::COLOR_POU_OUTLINE.b, 255};

    int outlineThickness = std::max(2, static_cast<int>(4 * scale));

    // Outer outline passes
    int numSteps = 40;
    for (int i = 0; i < numSteps; ++i) {
        float t = static_cast<float>(i) / numSteps;
        float y = (cy - baseH * 0.42f) + t * (baseH * 0.85f);
        // Top dome narrower, bottom dome wider
        float widthRatio = 0.55f + 0.55f * std::sin(t * 2.2f);
        int halfW = static_cast<int>((baseW * 0.48f * widthRatio));

        // Draw outline line
        SDL_SetRenderDrawColor(r, outlineColor.r, outlineColor.g, outlineColor.b, 255);
        SDL_RenderDrawLine(r, static_cast<int>(cx - halfW - outlineThickness), static_cast<int>(y),
                              static_cast<int>(cx + halfW + outlineThickness), static_cast<int>(y));

        // Draw inner body line
        SDL_SetRenderDrawColor(r, bodyColor.r, bodyColor.g, bodyColor.b, 255);
        SDL_RenderDrawLine(r, static_cast<int>(cx - halfW), static_cast<int>(y),
                              static_cast<int>(cx + halfW), static_cast<int>(y));
    }

    // Top head dome
    int headRadius = static_cast<int>(baseW * 0.28f);
    ResourceManager::fillCircle(r, static_cast<int>(cx), static_cast<int>(cy - baseH * 0.35f), headRadius, bodyColor);
    ResourceManager::drawCircle(r, static_cast<int>(cx), static_cast<int>(cy - baseH * 0.35f), headRadius + outlineThickness, outlineColor, outlineThickness);

    // Bottom base curves
    int botRadius = static_cast<int>(baseW * 0.32f);
    ResourceManager::fillCircle(r, static_cast<int>(cx - baseW * 0.25f), static_cast<int>(cy + baseH * 0.32f), botRadius, bodyColor);
    ResourceManager::fillCircle(r, static_cast<int>(cx + baseW * 0.25f), static_cast<int>(cy + baseH * 0.32f), botRadius, bodyColor);
    ResourceManager::fillCircle(r, static_cast<int>(cx), static_cast<int>(cy + baseH * 0.35f), static_cast<int>(baseW * 0.35f), bodyColor);
}

void Pou::drawEyes(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats) {
    float eyeW = 34.0f * scale;
    float eyeH = 42.0f * scale;
    float eyeSpacing = 24.0f * scale;
    float eyeY = cy - 14.0f * scale;

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color dark = {40, 40, 40, 255};
    int outlineT = std::max(2, static_cast<int>(3.0f * scale));

    if (m_isBlinking || stats.isAsleep) {
        // Closed eyes (sleeping / blinking eyelashes arc)
        SDL_SetRenderDrawColor(r, dark.r, dark.g, dark.b, 255);
        for (int side = -1; side <= 1; side += 2) {
            float ecx = cx + side * (eyeSpacing + eyeW * 0.5f);
            for (int t = -static_cast<int>(eyeW * 0.45f); t <= static_cast<int>(eyeW * 0.45f); ++t) {
                int arcY = static_cast<int>(eyeY + eyeH * 0.2f + (t * t) / (eyeW * 0.8f));
                SDL_RenderDrawLine(r, static_cast<int>(ecx + t), arcY, static_cast<int>(ecx + t), arcY + outlineT);
            }
        }
        return;
    }

    // Both eyes (left and right)
    for (int side = -1; side <= 1; side += 2) {
        float ecx = cx + side * (eyeSpacing + eyeW * 0.5f);
        float ecy = eyeY;

        // White sclera
        ResourceManager::fillCircle(r, static_cast<int>(ecx), static_cast<int>(ecy), static_cast<int>(eyeH * 0.48f), white);
        ResourceManager::drawCircle(r, static_cast<int>(ecx), static_cast<int>(ecy), static_cast<int>(eyeH * 0.48f), dark, outlineT);

        // Pupil
        float pupilX = ecx + m_currentPupilOffsetX * scale;
        float pupilY = ecy + m_currentPupilOffsetY * scale;
        float pupilR = 8.5f * scale;
        ResourceManager::fillCircle(r, static_cast<int>(pupilX), static_cast<int>(pupilY), static_cast<int>(pupilR), dark);

        // Pupil glint reflection
        float glintX = pupilX - 2.5f * scale;
        float glintY = pupilY - 2.5f * scale;
        float glintR = 2.8f * scale;
        ResourceManager::fillCircle(r, static_cast<int>(glintX), static_cast<int>(glintY), static_cast<int>(glintR), white);
    }
}

void Pou::drawMouth(SDL_Renderer* r, float cx, float cy, float scale) {
    float mouthY = cy + 22.0f * scale;
    SDL_Color mouthColor = {PouConfig::COLOR_MOUTH.r, PouConfig::COLOR_MOUTH.g, PouConfig::COLOR_MOUTH.b, 255};
    int thickness = std::max(2, static_cast<int>(3.0f * scale));

    switch (m_mouthState) {
        case MouthExpression::OPEN_FOR_FOOD: {
            // Open mouth for feeding
            int mouthR = static_cast<int>(12.0f * scale);
            ResourceManager::fillCircle(r, static_cast<int>(cx), static_cast<int>(mouthY), mouthR, mouthColor);
            ResourceManager::fillCircle(r, static_cast<int>(cx), static_cast<int>(mouthY + 2), static_cast<int>(mouthR * 0.65f), {180, 50, 50, 255});
            break;
        }
        case MouthExpression::EATING: {
            // Chewing oscillation
            float chew = std::abs(std::sin(m_eatingTimer * 14.0f));
            int mouthR = static_cast<int>((4.0f + chew * 8.0f) * scale);
            ResourceManager::fillCircle(r, static_cast<int>(cx), static_cast<int>(mouthY), mouthR, mouthColor);
            break;
        }
        case MouthExpression::SAYING_NO:
        case MouthExpression::FROWNING: {
            // Sad / frowning curve
            SDL_SetRenderDrawColor(r, mouthColor.r, mouthColor.g, mouthColor.b, 255);
            int mouthW = static_cast<int>(18.0f * scale);
            for (int dx = -mouthW; dx <= mouthW; ++dx) {
                int arcY = static_cast<int>(mouthY + 5.0f * scale - (dx * dx) / (mouthW * 1.5f));
                SDL_RenderDrawLine(r, static_cast<int>(cx + dx), arcY, static_cast<int>(cx + dx), arcY + thickness);
            }
            break;
        }
        case MouthExpression::VERY_HAPPY: {
            // Big open smile
            int mouthW = static_cast<int>(24.0f * scale);
            SDL_SetRenderDrawColor(r, mouthColor.r, mouthColor.g, mouthColor.b, 255);
            for (int dx = -mouthW; dx <= mouthW; ++dx) {
                int arcY = static_cast<int>(mouthY + (dx * dx) / (mouthW * 1.2f));
                SDL_RenderDrawLine(r, static_cast<int>(cx + dx), arcY, static_cast<int>(cx + dx), arcY + thickness + 3);
            }
            break;
        }
        case MouthExpression::NORMAL:
        case MouthExpression::HAPPY:
        default: {
            // Standard smile
            int mouthW = static_cast<int>(16.0f * scale);
            SDL_SetRenderDrawColor(r, mouthColor.r, mouthColor.g, mouthColor.b, 255);
            for (int dx = -mouthW; dx <= mouthW; ++dx) {
                int arcY = static_cast<int>(mouthY + (dx * dx) / (mouthW * 1.4f));
                SDL_RenderDrawLine(r, static_cast<int>(cx + dx), arcY, static_cast<int>(cx + dx), arcY + thickness);
            }
            break;
        }
    }
}

void Pou::drawDirt(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats) {
    if (!stats.isDirty && stats.dirtAccumulator < 100.0f) return;

    // Dirt spots scattered on body
    SDL_Color dirtColor = {80, 50, 20, 190};
    ResourceManager::fillCircle(r, static_cast<int>(cx - 35 * scale), static_cast<int>(cy + 25 * scale), static_cast<int>(7 * scale), dirtColor);
    ResourceManager::fillCircle(r, static_cast<int>(cx + 40 * scale), static_cast<int>(cy + 15 * scale), static_cast<int>(8 * scale), dirtColor);
    ResourceManager::fillCircle(r, static_cast<int>(cx - 15 * scale), static_cast<int>(cy + 42 * scale), static_cast<int>(6 * scale), dirtColor);
    ResourceManager::fillCircle(r, static_cast<int>(cx + 20 * scale), static_cast<int>(cy + 38 * scale), static_cast<int>(7 * scale), dirtColor);
}

void Pou::drawSoapBubbles(SDL_Renderer* r, float cx, float cy, float scale, const PetStats& stats) {
    if (stats.soapAccumulator <= 0.0f) return;

    // Frothy soap bubbles
    SDL_Color bubbleColor = {230, 245, 255, 220};
    SDL_Color bubbleRim = {180, 210, 245, 255};

    float numBubbles = std::min(15.0f, stats.soapAccumulator * 0.2f);
    for (int i = 0; i < static_cast<int>(numBubbles); ++i) {
        float angle = i * 2.399f;
        float dist = (15.0f + (i % 4) * 12.0f) * scale;
        int bx = static_cast<int>(cx + std::cos(angle) * dist);
        int by = static_cast<int>(cy + 15.0f * scale + std::sin(angle) * (dist * 0.7f));
        int br = static_cast<int>((6.0f + (i % 3) * 3.0f) * scale);

        ResourceManager::fillCircle(r, bx, by, br, bubbleColor);
        ResourceManager::drawCircle(r, bx, by, br, bubbleRim, 1);
    }
}

void Pou::drawHearts(SDL_Renderer* r) {
    for (const auto& h : m_hearts) {
        SDL_Color heartColor = {255, 80, 120, static_cast<uint8_t>(h.alpha * 255)};
        int hs = static_cast<int>(h.size);
        ResourceManager::fillCircle(r, static_cast<int>(h.x - hs * 0.35f), static_cast<int>(h.y), hs / 2, heartColor);
        ResourceManager::fillCircle(r, static_cast<int>(h.x + hs * 0.35f), static_cast<int>(h.y), hs / 2, heartColor);
    }
}
