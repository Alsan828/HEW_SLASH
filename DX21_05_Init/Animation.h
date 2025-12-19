#pragma once
#include "Render.h"   
#include <string>
#include <unordered_map>

struct AnimationClip
{
    std::string name;           // ¶¯»­Ãû³Æ
    int startFrame;            // ÆğÊ¼Ö¡Ë÷Òı
    int endFrame;              // ½áÊøÖ¡Ë÷Òı
    int splitX;                // Ë®Æ½·Ö¸ûæı
    int splitY;                // ´¹Ö±·Ö¸ûæı
    int currentFrame;          // µ±Ç°Ö¡
    int frameCount;            // ×ÜÖ¡Êı
    float frameTime;           // Ã¿Ö¡Ê±¼E
    float elapsedTime;         // ÒÑ¹ıÊ±¼E
    bool loop;                 // ÊÇ·ñÑ­»·
    ID3D11ShaderResourceView* textureSRV; // ÎÆÀúóÊÔ´ÊÓÍ¼

    // ¹¹ÔE¯Êı
    AnimationClip()
        : startFrame(0), endFrame(0), splitX(1), splitY(1)
        , currentFrame(0), frameCount(0), frameTime(0.1f)
        , elapsedTime(0.0f), loop(true), textureSRV(nullptr) {
    }

    // ³õÊ¼»¯º¯Êı
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

    // ¸EÂ¶¯»­
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

    // ÖØÖÃ¶¯»­
    void Reset()
    {
        currentFrame = startFrame;
        elapsedTime = 0.0f;
    }

    // »ñÈ¡UVÆ«ÒÆ
    DirectX::XMFLOAT2 GetUVOffset() const
    {
        return DirectX::XMFLOAT2(
            (float)(currentFrame % splitX) / splitX,
            (float)(currentFrame / splitX) / splitY
        );
    }

    // ¼EéÊÇ·ñ½áÊE
    bool IsFinished() const
    {
        return !loop && currentFrame == endFrame;
    }
};

class Animation
{
private:
    AnimationClip* m_currentClip;   // µ±Ç°¶¯»­Æ¬¶Î
    std::unordered_map<std::string, AnimationClip> m_clips; // ËùÓĞ¶¯»­Æ¬¶Î

    bool m_paused;  // ÊÇ·ñÔİÍ£
    DirectX::XMFLOAT2 m_uvOffset; // µ±Ç°UVÆ«ÒÆ

public:
    Animation(void);  // ¹¹ÔE¯Êı
    ~Animation(void); // Îö¹¹º¯Êı

    // ÌúØÓ¶¯»­Æ¬¶Î
    void AddClip(const std::string& name, int startFrame, int endFrame,
        int splitX, int splitY, float frameTime, bool loop,
        ID3D11ShaderResourceView* textureSRV = nullptr);

    // ÉèÖÃµ±Ç°¶¯»­Æ¬¶Î
    void SetClip(const std::string& name);

    // »ñÈ¡µ±Ç°¶¯»­Æ¬¶ÎÃû³Æ
    std::string GetCurrentClipName() const;

    // »ñÈ¡µ±Ç°¶¯»­Æ¬¶ÎÎÆÀE
    ID3D11ShaderResourceView* GetCurrentClipTexture() const;

    // ¸EÂ¶¯»­
    void Update(float deltaTime);

    // »ñÈ¡UVÆ«ÒÆ
    DirectX::XMFLOAT2 GetUVOffset(void) const;

    // ÖØÖÃµ±Ç°¶¯»­
    void Reset();

    // ¼EéÊÇ·ñ½áÊE
    bool IsFinished() const;

    // »ñÈ¡µ±Ç°Ö¡
    int GetCurrentFrame() const;

    // ÔİÍ£/»Ö¸´
    void Pause();
    void Resume();
    bool IsPaused() const;

    // »ñÈ¡·Ö¸ûìÅÏ¢
    int GetSplitX() const;
    int GetSplitY() const;

    // ¼Eé¶¯»­ÊÇ·ñ´æÔÚ
    bool HasClip(const std::string& name) const;

    // »ñÈ¡¶¯»­Æ¬¶ÎÊıÁ¿
    size_t GetClipCount() const;

    // Çå¿ÕËùÓĞ¶¯»­
    void ClearClips();
};