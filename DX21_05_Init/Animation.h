#pragma once
#include "Render.h"   
#include <string>
#include <unordered_map>


struct AnimationClip
{
    int startFrame;     // first frame index in the clip
    int endFrame;       // last frame index in the clip
    float frameTime;    // time per frame
    bool loop;          // should this clip loop?
};

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

    AnimationClip m_currentClip;   // for the current animation clip
    std::unordered_map<std::string, AnimationClip> m_clips; // for all the animation clips

    bool m_paused;  // if the game is paused or not


    // this is used for separate .png in order to have animation
    std::vector<ID3D11ShaderResourceView*> m_textures;
    bool m_useTextures = false;

public:
    Animation(void);  //construct

    HRESULT Init(int splitX, int splitY, float frameTime, int startFrame = 0);

    // so you can use it for idle, run, jump, dash, gameover, etc
    void AddClip(const std::string& name, int startFrame, int endFrame, float frameTime, bool loop);
    void SetClip(const std::string& name);
    std::string GetCurrentClipName() const; // gets the current clip name

    void Update(float deltaTime); // for updating the animation
    DirectX::XMFLOAT2 GetUVOffset(void) const; 

    void Reset();
    bool IsFinished() const;
    int GetCurrentFrame() const;

    void Pause();    // when the game is paused
    void Resume();   // when you resume the game
    bool IsPaused() const; // check if paused 

    int GetSplitX() const { return m_splitX; }
    int GetSplitY() const { return m_splitY; }



    // used for .png
    std::vector<float> m_frameTimes; // for duration per frame
    void InitFromTextures(const std::vector<ID3D11ShaderResourceView*>& textures,float frameTime, bool loop = false);
    ID3D11ShaderResourceView* GetCurrentTexture() const;
    void UpdateTexture(float deltaTime);
    void CleanupTextures();

};