#pragma once

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