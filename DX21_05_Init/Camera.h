#pragma once
#define NOMINMAX  // 必须在包含windows.h之前
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

    float m_zoomLevel = 1.0f; // 缩放级别

    int m_windowWidth, m_windowHeight; // 添加窗口尺寸记录


public:
    Camera();

    // 获取和设置缩放级别
    float GetZoom() const { return m_zoomLevel; }
    void SetZoom(float zoom) {
        m_zoomLevel = (std::max)(0.1f, (std::min)(zoom, 5.0f)); // 在函数名和括号之间加空格
    }

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


    // 获取屏幕中心的世界坐标
    void GetScreenCenterWorld(float& centerX, float& centerY) const {
        centerX = m_posX + (m_windowWidth * 0.5f) / (m_zoomLevel * 100.0f);
        centerY = m_posY + (m_windowHeight * 0.5f) / (m_zoomLevel * 100.0f);
    }

    // 世界坐标到屏幕坐标的转换
    void WorldToScreen(float worldX, float worldY, int& screenX, int& screenY) const {
        screenX = static_cast<int>((worldX - m_posX) * m_zoomLevel + m_windowWidth * 0.5f);
        screenY = static_cast<int>((worldY - m_posY) * m_zoomLevel + m_windowHeight * 0.5f);
    }

    // 屏幕坐标到世界坐标的转换
    void ScreenToWorld(int screenX, int screenY, float& worldX, float& worldY) const {
        worldX = (screenX - m_windowWidth * 0.5f) / m_zoomLevel + m_posX;
        worldY = (screenY - m_windowHeight * 0.5f) / m_zoomLevel + m_posY;
    }

    // 获取视口边界（世界坐标）
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



    // 修复后的可视范围计算（基于窗口尺寸和缩放）
    void GetVisibleRect(float& left, float& top, float& right, float& bottom) const {
        // 基于窗口尺寸和缩放计算实际可视范围
        float visibleWidth = m_windowWidth / m_zoomLevel;
        float visibleHeight = m_windowHeight / m_zoomLevel;

        float halfWidth = visibleWidth * 0.5f;
        float halfHeight = visibleHeight * 0.5f;

        // 修正坐标系：假设Y轴向上
        left = m_posX - halfWidth;
        right = m_posX + halfWidth;
        top = m_posY + halfHeight;     // 上边界（较大的Y值）
        bottom = m_posY - halfHeight;  // 下边界（较小的Y值）
    }

    // 修复后的可见性检测
    bool IsRectVisible(float x, float y, float width, float height) const {
        float camLeft, camTop, camRight, camBottom;
        GetVisibleRect(camLeft, camTop, camRight, camBottom);

        float objLeft = x;
        float objRight = x + width;
        float objTop = y + height;  // 假设物体原点在左下角
        float objBottom = y;

        // 正确的AABB相交测试[2](@ref)
        return !(objRight < camLeft ||
            objLeft > camRight ||
            objBottom > camTop ||
            objTop < camBottom);
    }

};
