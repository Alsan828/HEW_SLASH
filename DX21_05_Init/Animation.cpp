#include "Animation.h"

//this is for the inialize of the animationc consctruct
Animation::Animation() : m_splitX(1), m_splitY(1), m_frameCount(1), m_currentFrame(0),
m_frameTime(0.1f), m_elapsedTime(0.0f), m_uvOffset{ 0.0f, 0.0f } {}

HRESULT Animation::Init(int splitX, int splitY, float frameTime, int startFrame) 
{
    m_splitX = splitX;
    m_splitY = splitY;
    m_frameCount = splitX * splitY;
    m_frameTime = frameTime;
    m_currentFrame = startFrame;
    m_elapsedTime = 0.0f;

    // this is for the initial uv offset
    m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
    m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
    return S_OK;
}

void Animation::Update(float deltaTime) 
{
    m_elapsedTime += deltaTime;
    if (m_elapsedTime >= m_frameTime) 
    {
        m_currentFrame = (m_currentFrame + 1) % m_frameCount;
        m_elapsedTime = 0.0f;

        // this is for updating the uv offset
        m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
        m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
    }
}

DirectX::XMFLOAT2 Animation::GetUVOffset() const 
{
    return m_uvOffset;
}

void Animation::SetFrame(int frame) 
{
    m_currentFrame = frame % m_frameCount;
    m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
    m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
    m_elapsedTime = 0.0f;
}

void Animation::Reset() 
{
    m_currentFrame = 0;
    m_elapsedTime = 0.0f;
    m_uvOffset.x = 0.0f;
    m_uvOffset.y = 0.0f;
}