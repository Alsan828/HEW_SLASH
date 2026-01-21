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
    , m_deadZoneRadius(0.2f){


    m_zoomLevel = 1.0f;
	m_windowWidth = 1920;
	m_windowHeight = 1080;
}

void Camera::SetTarget(float x, float y) {
    m_targetX = x;
    m_targetY = y;
}

void Camera::Update(float deltaTime) {
    // Calculate player center position
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    // During charge, keep updating the stored mouse world position so camera bias follows cursor.
    if (g_player.isCharging) {
        float mx, my;
        g_inputSystem.GetMousePosition(mx, my);
        g_player.mouseTargetX = mx;
        g_player.mouseTargetY = my;
        g_player.hasMouseTarget = true;
    }

    // Make the player appear more centered on screen (less bottom-heavy framing)
    // Shift camera target upward a bit in world units.
    const float centerBiasY = 0.12f;
    playerCenterY += centerBiasY;

    // Apply look-ahead based on player movement direction
    float lookAheadX = 0.0f;
    float lookAheadY = 0.0f;

    if (fabsf(g_player.velocityX) > 0.1f || fabsf(g_player.velocityY) > 0.1f) {
        lookAheadX = g_player.velocityX * 0.5f * m_lookAheadFactor;
        lookAheadY = g_player.velocityY * 0.2f * m_lookAheadFactor;
    }

    // While charging, slightly bias camera towards mouse direction.
    // This makes aiming feel better without fully centering on the cursor.
    float chargeLookX = 0.0f;
    float chargeLookY = 0.0f;
    if (g_player.isCharging && g_player.hasMouseTarget) {
        // The mouse target is stored in world coords.
        float dxm = g_player.mouseTargetX - playerCenterX;
        float dym = g_player.mouseTargetY - playerCenterY;
        float len = sqrtf(dxm * dxm + dym * dym);
        if (len > 1e-4f) {
            dxm /= len;
            dym /= len;
        }

        // Amount in world units (tuned to be subtle).
        // Increase a bit with charge time but keep an upper bound.
        const float baseOffset = 0.10f;
        const float maxOffset = 0.22f;
        float t = 0.0f;
        if (g_player.CHARGE_THRESHOLD_LOW > 1e-4f) {
            t = std::clamp(g_player.chargeTime / g_player.CHARGE_THRESHOLD_LOW, 0.0f, 1.0f);
        }
        float offset = baseOffset + (maxOffset - baseOffset) * t;

        chargeLookX = dxm * offset;
        chargeLookY = dym * offset;
    }

    // Update target position with look-ahead
    m_targetX = playerCenterX + lookAheadX + chargeLookX;
    m_targetY = playerCenterY + lookAheadY + chargeLookY;

    // Calculate distance from current camera position to target
    float dx = m_targetX - m_posX;
    float dy = m_targetY - m_posY;
    float distance = sqrtf(dx * dx + dy * dy);

    // Only move camera if outside dead zone
    if (distance > m_deadZoneRadius) {
        // Apply smooth interpolation
        float smoothFactor = m_smoothSpeed * deltaTime * 60.0f; // Frame-rate independent

        // Distance-based boost: when the camera is far away, it catches up faster.
        // Close range keeps the original behavior.
        const float boostStartDist = m_deadZoneRadius * 2.0f;
        const float boostFullDist = m_deadZoneRadius * 10.0f;
        float boostT = 0.0f;
        if (boostFullDist > boostStartDist) {
            boostT = std::clamp((distance - boostStartDist) / (boostFullDist - boostStartDist), 0.0f, 1.0f);
        }
        float boostedSmoothFactor = smoothFactor * (1.0f + 2.5f * boostT);

        // Allow the camera to catch up faster.
        smoothFactor = std::clamp(boostedSmoothFactor, 0.01f, 0.98f); // Clamp to reasonable values

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
