#include "Animation.h"

//this is for the inialize of the animationc consctruct
Animation::Animation()
{
    m_splitX = 1;
    m_splitY = 1;
    m_frameCount = 1;
    m_currentFrame = 0;
    m_frameTime = 0.1f;
    m_elapsedTime = 0.0f;
    m_uvOffset = { 0.0f, 0.0f };

    // Default clip: single frame, loops
    m_currentClip = { 0, 0, 0.1f, true };
    m_clips = {}; 

    m_paused = false; // start with false when you start the game
}


HRESULT Animation::Init(int splitX, int splitY, float frameTime, int startFrame) 
{
    m_splitX = splitX;               // colums in the sprite sheet
    m_splitY = splitY;               // rows in the sprite sheet
    m_frameCount = splitX * splitY;  // total frames 
    m_currentFrame = 0;              // start frame default to 0
    m_elapsedTime = 0.0f;            // it resets it to 0
    m_uvOffset = { 0.0f, 0.0f };     // deault to 0

    return S_OK;
}

// for adding animation clip
void Animation::AddClip(const std::string& name, int startFrame, int endFrame, float frameTime, bool loop)
{
    AnimationClip clip{ startFrame, endFrame, frameTime, loop };  // it creates the clip
    m_clips[name] = clip;                                         // it stores the clip
}

// it switches the clip to the one we want
void Animation::SetClip(const std::string& name)
{
    auto it = m_clips.find(name); // looks for the clip
    // if it founds the clip
    if (it != m_clips.end())      
    {
        m_currentClip = it->second; // it sets the clip as the current
        m_currentFrame = m_currentClip.startFrame; // resets it to the start frame
        m_elapsedTime = 0.0f;  // it resets the timer

        // it calculates the uv offset for the frame
        m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
        m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
    }
}

// it gets the current clip name and it return the name of it
std::string Animation::GetCurrentClipName() const
{
    for (const auto& kv : m_clips)
    {
        if (kv.second.startFrame == m_currentClip.startFrame &&
            kv.second.endFrame == m_currentClip.endFrame)
        {
            return kv.first;
        }
    }

    return "";
}

// it update animation based on elapsed time
void Animation::Update(float deltaTime) 
{
    if (m_paused)
    {
        return; // do nothing if the games is paused
    }

    m_elapsedTime += deltaTime;
    if (m_elapsedTime >= m_currentClip.frameTime)
    {
        m_elapsedTime = 0.0f;  // reset timer

        if (m_currentFrame < m_currentClip.endFrame)
        {
            m_currentFrame++; // go to next frame
        }
        else
        {
            if (m_currentClip.loop)
            {
                m_currentFrame = m_currentClip.startFrame;
            }
        }

        // it updates UV offset for new frame
        m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
        m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
    }
}


DirectX::XMFLOAT2 Animation::GetUVOffset() const 
{
    return m_uvOffset; // it returns UV offset for current frame
}

// it resets current clip back to start frame
void Animation::Reset()
{
    m_currentFrame = m_currentClip.startFrame;
    m_elapsedTime = 0.0f;
    m_uvOffset.x = (float)(m_currentFrame % m_splitX) / m_splitX;
    m_uvOffset.y = (float)(m_currentFrame / m_splitX) / m_splitY;
}

// Check if non-looping animation has finished
bool Animation::IsFinished() const
{
    //return (!m_currentClip.loop && m_currentFrame == m_currentClip.endFrame);

    if (m_useTextures) {
        // flipbook mode: finished when we reached the last texture
        return (m_currentFrame == m_frameCount - 1 && m_paused);
    }
    else {
        // sprite sheet mode: keep your original logic
        return (!m_currentClip.loop &&
            m_currentFrame == m_currentClip.endFrame &&
            m_paused);
    }
}

// it gets the current frame of the aniamtion
int Animation::GetCurrentFrame() const
{
    return m_currentFrame;
}

// for pausing the game
void Animation::Pause()
{
    m_paused = true;
}

// for resuming the game
void Animation::Resume()
{
    m_paused = false;
}

// check if its paused or not
bool Animation::IsPaused() const
{
    return m_paused;
}



// used for the animation with .png
void Animation::InitFromTextures(const std::vector<ID3D11ShaderResourceView*>& textures, float frameTime, bool loop)
{
    m_textures = textures;
    m_frameCount = (int)textures.size();
    m_currentFrame = 0;
    m_frameTime = frameTime;
    m_elapsedTime = 0.0f;
    m_useTextures = true;
    m_paused = true;

}


ID3D11ShaderResourceView* Animation::GetCurrentTexture() const
{
    if (m_useTextures && !m_textures.empty()) {
        return m_textures[m_currentFrame];
    }
    return nullptr;
}

void Animation::UpdateTexture(float deltaTime)
{
    if (m_paused) return;

    m_elapsedTime += deltaTime;
    if (m_elapsedTime >= m_frameTime)
    {
        m_elapsedTime = 0.0f;
        m_currentFrame++;

        if (m_currentFrame >= m_frameCount)
        {
            if (m_useTextures)
            {
                m_currentFrame = m_frameCount - 1; // stay on last frame
                m_paused = true; // stop animation
            }
            else if (m_currentClip.loop) {
                m_currentFrame = m_currentClip.startFrame;
            }
            else {
                m_currentFrame = m_currentClip.endFrame;
                m_paused = true;
            }
        }
    }
}

void Animation::CleanupTextures()
 {
     for (auto tex : m_textures) {
         if (tex) {
             tex->Release();
         }
     }
     m_textures.clear();
 }







//=======================================
// THIS IS HOW YOU WOULD USE IT IN OTHER .CPP
// =======================================
//Animation anim;
//anim.Init(4, 4); // 4x4 sprite sheet
//
//// Add clips
//anim.AddClip("Idle", 0, 5, 0.2f, true);
//anim.AddClip("Run", 6, 12, 0.1f, true);
//anim.AddClip("Jump", 13, 16, 0.15f, false);
//
//// Switch based on state
//anim.SetClip("Run");
//anim.Update(deltaTime);
//DirectX::XMFLOAT2 uv = anim.GetUVOffset();