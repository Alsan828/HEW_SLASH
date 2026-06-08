#include "Camera.h"
#include "Game.h"

// カメラ実装
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
    // プレイヤー中心座標を計算する
    float playerCenterX = g_player.posX + PLAYER_WIDTH * 0.5f;
    float playerCenterY = g_player.posY + PLAYER_HEIGHT * 0.5f;

    // チャージ中は保存済みのマウスワールド座標を更新し、カメラの偏りがカーソルに追従するようにする。
    if (g_player.isCharging) {
        float mx, my;
        g_inputSystem.GetMousePosition(mx, my);
        g_player.mouseTargetX = mx;
        g_player.mouseTargetY = my;
        g_player.hasMouseTarget = true;
    }

    // プレイヤーが画面中央寄りに見えるようにする（下寄りの構図を弱める）。
    // カメラの目標位置をワールド座標で少し上へずらす。
    const float centerBiasY = 0.12f;
    playerCenterY += centerBiasY;

    // プレイヤーの移動方向に応じて先読みオフセットを適用する
    float lookAheadX = 0.0f;
    float lookAheadY = 0.0f;

    if (fabsf(g_player.velocityX) > 0.1f || fabsf(g_player.velocityY) > 0.1f) {
        lookAheadX = g_player.velocityX * 0.5f * m_lookAheadFactor;
        lookAheadY = g_player.velocityY * 0.2f * m_lookAheadFactor;
    }

    // チャージ中はカメラをマウス方向へ少しだけ寄せる。
    // カーソルを完全に中央にしなくても狙いやすくなる。
    float chargeLookX = 0.0f;
    float chargeLookY = 0.0f;
    if (g_player.isCharging && g_player.hasMouseTarget) {
        // マウス目標位置はワールド座標で保存されている。
        float dxm = g_player.mouseTargetX - playerCenterX;
        float dym = g_player.mouseTargetY - playerCenterY;
        float len = sqrtf(dxm * dxm + dym * dym);
        if (len > 1e-4f) {
            dxm /= len;
            dym /= len;
        }

        // ワールド座標系でのオフセット量（控えめになるよう調整済み）。
        // チャージ時間に応じて増やすが、上限は超えない。
        // 要望により、チャージ中のカメラオフセットを強めている。
        const float baseOffset = 0.20f;
        const float maxOffset = 0.44f;
        float t = 0.0f;
        if (g_player.CHARGE_THRESHOLD_LOW > 1e-4f) {
            t = std::clamp(g_player.chargeTime / g_player.CHARGE_THRESHOLD_LOW, 0.0f, 1.0f);
        }
        float offset = baseOffset + (maxOffset - baseOffset) * t;

        chargeLookX = dxm * offset;
        chargeLookY = dym * offset;
    }

    // 先読みオフセットを反映して目標位置を更新する
    m_targetX = playerCenterX + lookAheadX + chargeLookX;
    m_targetY = playerCenterY + lookAheadY + chargeLookY;

    // 現在のカメラ位置から目標までの距離を計算する
    float dx = m_targetX - m_posX;
    float dy = m_targetY - m_posY;
    float distance = sqrtf(dx * dx + dy * dy);

    // デッドゾーンの外にいる場合のみカメラを動かす
    if (distance > m_deadZoneRadius) {
        // スムーズ補間を適用する
        float smoothFactor = m_smoothSpeed * deltaTime * 60.0f; // フレームレート非依存

        // 距離に応じた加速：遠いほど早く追いつくようにする。
        // 近距離では元の挙動を維持する。
        const float boostStartDist = m_deadZoneRadius * 2.0f;
        const float boostFullDist = m_deadZoneRadius * 10.0f;
        float boostT = 0.0f;
        if (boostFullDist > boostStartDist) {
            boostT = std::clamp((distance - boostStartDist) / (boostFullDist - boostStartDist), 0.0f, 1.0f);
        }
        float boostedSmoothFactor = smoothFactor * (1.0f + 2.5f * boostT);

        // カメラがより早く追いつけるようにする。
        smoothFactor = std::clamp(boostedSmoothFactor, 0.01f, 0.98f); // 妥当な範囲に制限する

        m_posX += dx * smoothFactor;
        m_posY += dy * smoothFactor;
    }

    // カメラシェイクが有効なら適用する
    if (m_shakeTimer > 0.0f) {
        m_shakeTimer -= deltaTime;

        // 時間経過とともに強度を弱める
        m_shakeIntensity *= 0.9f;

        if (m_shakeTimer <= 0.0f) {
            m_shakeTimer = 0.0f;
            m_shakeIntensity = 0.0f;
        }
        else {
            // シェイク用のランダムオフセットを適用する
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
