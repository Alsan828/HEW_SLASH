#pragma once
#include "Render.h"   
#include <string>
#include <unordered_map>

struct AnimationClip
{
    std::string name;           // ��������
    int startFrame;            // ��ʼ֡����
    int endFrame;              // ����֡����
    int splitX;                // ˮƽ�ָ���
    int splitY;                // ��ֱ�ָ���
    int currentFrame;          // ��ǰ֡
    int frameCount;            // ��֡��
    float frameTime;           // ÿ֡ʱ��E
    float elapsedTime;         // �ѹ�ʱ��E
    bool loop;                 // �Ƿ�ѭ��
    ID3D11ShaderResourceView* textureSRV; // ������Դ��ͼ

    AnimationClip()
        : startFrame(0), endFrame(0), splitX(1), splitY(1)
        , currentFrame(0), frameCount(0), frameTime(0.1f)
        , elapsedTime(0.0f), loop(true), textureSRV(nullptr) {
    }

    void Init(const std::string& clipName, int sFrame, int eFrame,
        int sX, int sY, float fTime, bool l, ID3D11ShaderResourceView* tex)
    {
        name = clipName;
        startFrame = sFrame;
        endFrame = eFrame;
        splitX = sX;
        splitY = sY;
        frameTime = fTime;
        loop = l;
        textureSRV = tex;
        currentFrame = startFrame;
        frameCount = splitX * splitY;
        elapsedTime = 0.0f;
    }

    void Update(float deltaTime)
    {
        elapsedTime += deltaTime;
        if (elapsedTime >= frameTime)
        {
            elapsedTime = 0.0f;

            if (currentFrame < endFrame)
            {
                currentFrame++;
            }
            else if (loop)
            {
                currentFrame = startFrame;
            }
        }
    }

    void Reset()
    {
        currentFrame = startFrame;
        elapsedTime = 0.0f;
    }

    DirectX::XMFLOAT2 GetUVOffset() const
    {
        return DirectX::XMFLOAT2(
            (float)(currentFrame % splitX) / splitX,
            (float)(currentFrame / splitX) / splitY
        );
    }

    bool IsFinished() const
    {
        return !loop && currentFrame == endFrame;
    }
};

class Animation
{
private:
    AnimationClip* m_currentClip;  
    std::unordered_map<std::string, AnimationClip> m_clips; 

    bool m_paused; 
    DirectX::XMFLOAT2 m_uvOffset; 

public:
    Animation(void);  
    ~Animation(void); 

    void AddClip(const std::string& name, int startFrame, int endFrame,
        int splitX, int splitY, float frameTime, bool loop,
        ID3D11ShaderResourceView* textureSRV = nullptr);

    void SetClip(const std::string& name);

    std::string GetCurrentClipName() const;

    ID3D11ShaderResourceView* GetCurrentClipTexture() const;

    void Update(float deltaTime);

    DirectX::XMFLOAT2 GetUVOffset(void) const;

    void Reset();

    bool IsFinished() const;

    int GetCurrentFrame() const;

    void Pause();
    void Resume();
    bool IsPaused() const;

    int GetSplitX() const;
    int GetSplitY() const;

    bool HasClip(const std::string& name) const;

    size_t GetClipCount() const;

    void ClearClips();
};