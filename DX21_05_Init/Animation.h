#pragma once
//#include <DirectXMath.h>
//#include <Windows.h>
#include "Render.h"   


class Animation 
{
private:
    int m_splitX;           // for the horizontal sprite sheet divisions
    int m_splitY;           // for the vertical sprite sheet divisions
    int m_frameCount;       // for the total frames in the animation
    int m_currentFrame;     // for the current frame
    float m_frameTime;      // for the time per frame in seconds
    float m_elapsedTime;    // for the time since last the frame switched
    DirectX::XMFLOAT2 m_uvOffset; // for the uv offset for current frame

public:
    Animation(void);  //construct

    HRESULT Init(int splitX, int splitY, float frameTime, int startFrame = 0);
    void Update(float deltaTime);
    DirectX::XMFLOAT2 GetUVOffset(void) const;
    void SetFrame(int frame);
    void Reset(void);
    int GetFrameCount(void) const { return m_frameCount; }
};