#pragma once
#include "Render.h"   
#include <string>
#include <unordered_map>

struct AnimationClip
{
    std::string name;           // 动画名称
    int startFrame;            // 起始帧索引
    int endFrame;              // 结束帧索引
    int splitX;                // 水平分割数
    int splitY;                // 垂直分割数
    int currentFrame;          // 当前帧
    int frameCount;            // 总帧数
    float frameTime;           // 每帧时间
    float elapsedTime;         // 已过时间
    bool loop;                 // 是否循环
    ID3D11ShaderResourceView* textureSRV; // 纹理资源视图

    // 构造函数
    AnimationClip()
        : startFrame(0), endFrame(0), splitX(1), splitY(1)
        , currentFrame(0), frameCount(0), frameTime(0.1f)
        , elapsedTime(0.0f), loop(true), textureSRV(nullptr) {
    }

    // 初始化函数
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

    // 更新动画
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

    // 重置动画
    void Reset()
    {
        currentFrame = startFrame;
        elapsedTime = 0.0f;
    }

    // 获取UV偏移
    DirectX::XMFLOAT2 GetUVOffset() const
    {
        return DirectX::XMFLOAT2(
            (float)(currentFrame % splitX) / splitX,
            (float)(currentFrame / splitX) / splitY
        );
    }

    // 检查是否结束
    bool IsFinished() const
    {
        return !loop && currentFrame == endFrame;
    }
};

class Animation
{
private:
    AnimationClip* m_currentClip;   // 当前动画片段
    std::unordered_map<std::string, AnimationClip> m_clips; // 所有动画片段

    bool m_paused;  // 是否暂停
    DirectX::XMFLOAT2 m_uvOffset; // 当前UV偏移

public:
    Animation(void);  // 构造函数
    ~Animation(void); // 析构函数

    // 添加动画片段
    void AddClip(const std::string& name, int startFrame, int endFrame,
        int splitX, int splitY, float frameTime, bool loop,
        ID3D11ShaderResourceView* textureSRV = nullptr);

    // 设置当前动画片段
    void SetClip(const std::string& name);

    // 获取当前动画片段名称
    std::string GetCurrentClipName() const;

    // 获取当前动画片段纹理
    ID3D11ShaderResourceView* GetCurrentClipTexture() const;

    // 更新动画
    void Update(float deltaTime);

    // 获取UV偏移
    DirectX::XMFLOAT2 GetUVOffset(void) const;

    // 重置当前动画
    void Reset();

    // 检查是否结束
    bool IsFinished() const;

    // 获取当前帧
    int GetCurrentFrame() const;

    // 暂停/恢复
    void Pause();
    void Resume();
    bool IsPaused() const;

    // 获取分割信息
    int GetSplitX() const;
    int GetSplitY() const;

    // 检查动画是否存在
    bool HasClip(const std::string& name) const;

    // 获取动画片段数量
    size_t GetClipCount() const;

    // 清空所有动画
    void ClearClips();
};