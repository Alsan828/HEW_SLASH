#pragma once
#include <cmath>

#if __cplusplus < 201703L
namespace std {
    template<typename T>
    constexpr const T& clamp(const T& value, const T& low, const T& high) {
        return (value < low) ? low : (value > high) ? high : value;
    }
}
#endif

extern float camera_Smoothness;
extern float camera_LookAhead;
extern float camera_DeadZone;

// Camera class for smooth player tracking
class Camera {
private:
    float m_posX, m_posY;           // Current camera position
    float m_targetX, m_targetY;     // Target position (player center)
    float m_smoothSpeed;            // Smoothing factor (0-1, lower = smoother)
    float m_lerpFactor;             // Linear interpolation factor
    float m_shakeTimer;             // Camera shake timer
    float m_shakeIntensity;         // Camera shake intensity
    float m_lookAheadFactor;       // Look ahead factor for player movement direction
    float m_deadZoneRadius;        // Dead zone radius where camera doesn't move

public:
    Camera();

    void SetTarget(float x, float y);
    void Update(float deltaTime);
    void Shake(float intensity, float duration);

    // Getters
    float GetX() const { return m_posX; }
    float GetY() const { return m_posY; }
    float GetShakeIntensity() const { return m_shakeIntensity; }

    // Camera configuration
    void SetSmoothness(float smoothness) { m_smoothSpeed = std::clamp(smoothness, 0.01f, 1.0f); }
    void SetLookAhead(float lookAhead) { m_lookAheadFactor = std::clamp(lookAhead, 0.0f, 1.0f); }
    void SetDeadZone(float radius) { m_deadZoneRadius = (radius > 0.0f) ? radius : 0.0f; }
};