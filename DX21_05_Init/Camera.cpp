#include "Camera.h"
#include "Game.h"

// Camera implementation
Camera::Camera()
    : m_posX(0.0f), m_posY(0.0f)
    , m_targetX(0.0f), m_targetY(0.0f)
    , m_smoothSpeed(0.1f)
    , m_lerpFactor(0.1f)
    , m_shakeTimer(0.0f)
    , m_shakeIntensity(0.0f)
    , m_lookAheadFactor(0.3f)
    , m_deadZoneRadius(0.2f) {
}

void Camera::SetTarget(float x, float y) {
    m_targetX = x;
    m_targetY = y;
}

void Camera::Update(float deltaTime) {
    // Calculate player center position
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    // Apply look-ahead based on player movement direction
    float lookAheadX = 0.0f;
    float lookAheadY = 0.0f;

    if (fabsf(g_player.velocityX) > 0.1f || fabsf(g_player.velocityY) > 0.1f) {
        lookAheadX = g_player.velocityX * 0.5f * m_lookAheadFactor;
        lookAheadY = g_player.velocityY * 0.2f * m_lookAheadFactor;
    }

    // Update target position with look-ahead
    m_targetX = playerCenterX + lookAheadX;
    m_targetY = playerCenterY + lookAheadY;

    // Calculate distance from current camera position to target
    float dx = m_targetX - m_posX;
    float dy = m_targetY - m_posY;
    float distance = sqrtf(dx * dx + dy * dy);

    // Only move camera if outside dead zone
    if (distance > m_deadZoneRadius) {
        // Apply smooth interpolation
        float smoothFactor = m_smoothSpeed * deltaTime * 60.0f; // Frame-rate independent
        smoothFactor = std::clamp(smoothFactor, 0.01f, 0.5f); // Clamp to reasonable values

        m_posX += dx * smoothFactor;
        m_posY += dy * smoothFactor;
    }

    // Apply camera shake if active
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= deltaTime;

        // Reduce intensity over time
        m_shakeIntensity *= 0.9f;

        if (m_shakeTimer <= 0.0f) {
            m_shakeTimer = 0.0f;
            m_shakeIntensity = 0.0f;
        }
        else {
            // Apply random offset for shake effect
            float shakeX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * m_shakeIntensity;
            float shakeY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * m_shakeIntensity;

            m_posX += shakeX;
            m_posY += shakeY;
        }
    }
}

void Camera::Shake(float intensity, float duration) {
    m_shakeIntensity = intensity;
    m_shakeTimer = duration;
}
