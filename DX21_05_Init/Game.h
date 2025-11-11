#pragma once

#include <cstdint>
#include "InputSystem.h"

// Remove original IsKeyDown function declaration
// Add input system instance declaration
extern InputSystem g_inputSystem;

class GameTimer {
private:
    __int64 m_prevTime = 0;
    __int64 m_currTime = 0;
    double m_secondsPerCount = 0.0;
    float m_deltaTime = 0.0f;

public:
    GameTimer();
    void Tick();
    float GetDeltaTime() const;
};

// Game initialization
void InitGameWorld();

// Game loop
void GameLoop();

// Drawing function
void Draw();

// Input handling
void HandleInput();

// Reset game
void ResetGame();