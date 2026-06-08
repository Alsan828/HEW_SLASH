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

// アニメーションクリップを追加
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

// 現在のアニメーションクリップを設定
void Animation::SetClip(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        m_currentClip = &(it->second);
        m_currentClip->Reset();
    }
}

// 現在のアニメーションクリップ名を取得
std::string Animation::GetCurrentClipName() const
{
    if (m_currentClip)
    {
        return m_currentClip->name;
    }
    return "";
}

// 現在のクリップのテクスチャを取得
ID3D11ShaderResourceView* Animation::GetCurrentClipTexture() const
{
    if (m_currentClip)
    {
        return m_currentClip->textureSRV;
    }
    return nullptr;
}

// アニメーションを更新
void Animation::Update(float deltaTime)
{
    if (m_paused || !m_currentClip)
    {
        return;
    }

    m_currentClip->Update(deltaTime);
    m_uvOffset = m_currentClip->GetUVOffset();
}

// UVオフセットを取得
DirectX::XMFLOAT2 Animation::GetUVOffset() const
{
    return m_uvOffset;
}

// 現在のアニメーションをリセット
void Animation::Reset()
{
    if (m_currentClip)
    {
        m_currentClip->Reset();
    }
}

// 現在のアニメーションが終了したかを確認
bool Animation::IsFinished() const
{
    if (m_currentClip)
    {
        return m_currentClip->IsFinished();
    }
    return true;
}

// 現在のフレームを取得
int Animation::GetCurrentFrame() const
{
    if (m_currentClip)
    {
        return m_currentClip->currentFrame;
    }
    return 0;
}

// アニメーションを一時停止
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

// 分割情報を取得
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

// 指定したクリップが存在するかを確認
bool Animation::HasClip(const std::string& name) const
{
    return m_clips.find(name) != m_clips.end();
}

// アニメーションクリップ数を取得
size_t Animation::GetClipCount() const
{
    return m_clips.size();
}

// すべてのアニメーションをクリア
void Animation::ClearClips()
{
    m_clips.clear();
    m_currentClip = nullptr;
}