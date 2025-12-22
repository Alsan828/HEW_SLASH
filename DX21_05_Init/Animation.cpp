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

// 铁赜动画片段
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

// 设置当前动画片段
void Animation::SetClip(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        m_currentClip = &(it->second);
        m_currentClip->Reset();
    }
}

// 获取当前动画片段名称
std::string Animation::GetCurrentClipName() const
{
    if (m_currentClip)
    {
        return m_currentClip->name;
    }
    return "";
}

// 获取当前动画片段纹纴E
ID3D11ShaderResourceView* Animation::GetCurrentClipTexture() const
{
    if (m_currentClip)
    {
        return m_currentClip->textureSRV;
    }
    return nullptr;
}

// 竵E露?
void Animation::Update(float deltaTime)
{
    if (m_paused || !m_currentClip)
    {
        return;
    }

    m_currentClip->Update(deltaTime);
    m_uvOffset = m_currentClip->GetUVOffset();
}

// 获取UV偏移
DirectX::XMFLOAT2 Animation::GetUVOffset() const
{
    return m_uvOffset;
}

// 重置当前动画
void Animation::Reset()
{
    if (m_currentClip)
    {
        m_currentClip->Reset();
    }
}

// 紒E槭欠窠崾丒
bool Animation::IsFinished() const
{
    if (m_currentClip)
    {
        return m_currentClip->IsFinished();
    }
    return true;
}

// 获取当前帧
int Animation::GetCurrentFrame() const
{
    if (m_currentClip)
    {
        return m_currentClip->currentFrame;
    }
    return 0;
}

// 暂停/恢复
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

// 获取分耕炫息
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

// 紒E槎欠翊嬖?
bool Animation::HasClip(const std::string& name) const
{
    return m_clips.find(name) != m_clips.end();
}

// 获取动画片段数量
size_t Animation::GetClipCount() const
{
    return m_clips.size();
}

// 清空所有动画
void Animation::ClearClips()
{
    m_clips.clear();
    m_currentClip = nullptr;
}