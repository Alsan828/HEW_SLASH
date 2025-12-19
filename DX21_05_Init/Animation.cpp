#include "Animation.h"

Animation::Animation()
{
    m_currentClip = nullptr;
    m_paused = false;
    m_uvOffset = DirectX::XMFLOAT2(0.0f, 0.0f);
}

Animation::~Animation()
{
    ClearClips();
    m_currentClip = nullptr;
}

// ÌúØÓ¶¯»­Æ¬¶Î
void Animation::AddClip(const std::string& name, int startFrame, int endFrame,
    int splitX, int splitY, float frameTime, bool loop,
    ID3D11ShaderResourceView* textureSRV)
{
    AnimationClip clip;
    clip.Init(name, startFrame, endFrame, splitX, splitY, frameTime, loop, textureSRV);
    m_clips[name] = clip;

    if (m_currentClip == nullptr)
    {
        m_currentClip = &m_clips[name];
    }
}

// ÉèÖÃµ±Ç°¶¯»­Æ¬¶Î
void Animation::SetClip(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        m_currentClip = &(it->second);
        m_currentClip->Reset();
    }
}

// »ñÈ¡µ±Ç°¶¯»­Æ¬¶ÎÃû³Æ
std::string Animation::GetCurrentClipName() const
{
    if (m_currentClip)
    {
        return m_currentClip->name;
    }
    return "";
}

// »ñÈ¡µ±Ç°¶¯»­Æ¬¶ÎÎÆÀE
ID3D11ShaderResourceView* Animation::GetCurrentClipTexture() const
{
    if (m_currentClip)
    {
        return m_currentClip->textureSRV;
    }
    return nullptr;
}

// ¸EÂ¶¯»­
void Animation::Update(float deltaTime)
{
    if (m_paused || !m_currentClip)
    {
        return;
    }

    m_currentClip->Update(deltaTime);
    m_uvOffset = m_currentClip->GetUVOffset();
}

// »ñÈ¡UVÆ«ÒÆ
DirectX::XMFLOAT2 Animation::GetUVOffset() const
{
    return m_uvOffset;
}

// ÖØÖÃµ±Ç°¶¯»­
void Animation::Reset()
{
    if (m_currentClip)
    {
        m_currentClip->Reset();
    }
}

// ¼EéÊÇ·ñ½áÊE
bool Animation::IsFinished() const
{
    if (m_currentClip)
    {
        return m_currentClip->IsFinished();
    }
    return true;
}

// »ñÈ¡µ±Ç°Ö¡
int Animation::GetCurrentFrame() const
{
    if (m_currentClip)
    {
        return m_currentClip->currentFrame;
    }
    return 0;
}

// ÔİÍ£/»Ö¸´
void Animation::Pause()
{
    m_paused = true;
}

void Animation::Resume()
{
    m_paused = false;
}

bool Animation::IsPaused() const
{
    return m_paused;
}

// »ñÈ¡·Ö¸ûìÅÏ¢
int Animation::GetSplitX() const
{
    if (m_currentClip)
    {
        return m_currentClip->splitX;
    }
    return 1;
}

int Animation::GetSplitY() const
{
    if (m_currentClip)
    {
        return m_currentClip->splitY;
    }
    return 1;
}

// ¼Eé¶¯»­ÊÇ·ñ´æÔÚ
bool Animation::HasClip(const std::string& name) const
{
    return m_clips.find(name) != m_clips.end();
}

// »ñÈ¡¶¯»­Æ¬¶ÎÊıÁ¿
size_t Animation::GetClipCount() const
{
    return m_clips.size();
}

// Çå¿ÕËùÓĞ¶¯»­
void Animation::ClearClips()
{
    m_clips.clear();
    m_currentClip = nullptr;
}