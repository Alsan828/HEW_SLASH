#pragma once
#define NOMINMAX  // windows.h を含める前に定義する必要がある
#include <windows.h>
#include <cmath>
#include <algorithm>

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

// プレイヤーを滑らかに追従するカメラクラス
class Camera {
private:
    float m_posX, m_posY;           // 現在のカメラ位置
    float m_targetX, m_targetY;     // 目標位置（プレイヤー中心）
    float m_smoothSpeed;            // スムージング係数（0-1、小さいほど滑らか）
    float m_lerpFactor;             // 線形補間係数
    float m_shakeTimer;             // カメラシェイクのタイマー
    float m_shakeIntensity;         // カメラシェイクの強度
    float m_lookAheadFactor;       // プレイヤー移動方向に対する先読み係数
    float m_deadZoneRadius;        // カメラが動かないデッドゾーン半径

    float m_zoomLevel = 1.0f; // ズーム倍率

    int m_windowWidth, m_windowHeight; // ウィンドウサイズを保持する


public:
    Camera();

    // ズーム倍率の取得と設定
    float GetZoom() const { return m_zoomLevel; }
    void SetZoom(float zoom) {
        m_zoomLevel = (std::max)(0.1f, (std::min)(zoom, 5.0f)); // ズーム値を有効範囲に制限する
    }

    void SetTarget(float x, float y);
    void Update(float deltaTime);
    void Shake(float intensity, float duration);

    // ゲッター
    float GetX() const { return m_posX; }
    float GetY() const { return m_posY; }
    float GetShakeIntensity() const { return m_shakeIntensity; }

    // カメラ設定
    void SetSmoothness(float smoothness) { m_smoothSpeed = std::clamp(smoothness, 0.01f, 1.0f); }
    void SetLookAhead(float lookAhead) { m_lookAheadFactor = std::clamp(lookAhead, 0.0f, 1.0f); }
    void SetDeadZone(float radius) { m_deadZoneRadius = (radius > 0.0f) ? radius : 0.0f; }


    // 画面中央のワールド座標を取得する
    void GetScreenCenterWorld(float& centerX, float& centerY) const {
        centerX = m_posX + (m_windowWidth * 0.5f) / (m_zoomLevel * 100.0f);
        centerY = m_posY + (m_windowHeight * 0.5f) / (m_zoomLevel * 100.0f);
    }

    // ワールド座標をスクリーン座標へ変換する
    void WorldToScreen(float worldX, float worldY, int& screenX, int& screenY) const {
        screenX = static_cast<int>((worldX - m_posX) * m_zoomLevel + m_windowWidth * 0.5f);
        screenY = static_cast<int>((worldY - m_posY) * m_zoomLevel + m_windowHeight * 0.5f);
    }

    // スクリーン座標をワールド座標へ変換する
    void ScreenToWorld(int screenX, int screenY, float& worldX, float& worldY) const {
        worldX = (screenX - m_windowWidth * 0.5f) / m_zoomLevel + m_posX;
        worldY = (screenY - m_windowHeight * 0.5f) / m_zoomLevel + m_posY;
    }

    // ビューポート境界をワールド座標で取得する
    void GetViewportBounds(float& left, float& right, float& top, float& bottom) const {
        left = m_posX - (m_windowWidth * 0.5f) / m_zoomLevel;
        right = m_posX + (m_windowWidth * 0.5f) / m_zoomLevel;
        top = m_posY - (m_windowHeight * 0.5f) / m_zoomLevel;
        bottom = m_posY + (m_windowHeight * 0.5f) / m_zoomLevel;
    }

    int GetWidth() const { return m_windowWidth; }
    int GetHeight() const { return m_windowHeight; }
    void SetWindowSize(int width, int height) {
        m_windowWidth = width;
        m_windowHeight = height;
    }



    // 修正後の可視範囲計算（ウィンドウサイズとズームに基づく）
    void GetVisibleRect(float& left, float& top, float& right, float& bottom) const {
        // ウィンドウサイズとズームから実際の可視範囲を計算する
        float visibleWidth = m_windowWidth / m_zoomLevel;
        float visibleHeight = m_windowHeight / m_zoomLevel;

        float halfWidth = visibleWidth * 0.5f;
        float halfHeight = visibleHeight * 0.5f;

        // 座標系を補正する：Y 軸は上向きと仮定する
        left = m_posX - halfWidth;
        right = m_posX + halfWidth;
        top = m_posY + halfHeight;     // 上境界（大きい Y 値）
        bottom = m_posY - halfHeight;  // 下境界（小さい Y 値）
    }

    // 修正後の可視判定
    bool IsRectVisible(float x, float y, float width, float height) const {
        float camLeft, camTop, camRight, camBottom;
        GetVisibleRect(camLeft, camTop, camRight, camBottom);

        float objLeft = x;
        float objRight = x + width;
        float objTop = y + height;  // オブジェクトの原点は左下と仮定する
        float objBottom = y;

        // 正しい AABB 交差判定
        return !(objRight < camLeft ||
            objLeft > camRight ||
            objBottom > camTop ||
            objTop < camBottom);
    }

};
