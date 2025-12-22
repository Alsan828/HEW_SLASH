// SimpleAudio.h
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#pragma comment(lib, "winmm.lib")

class SimpleAudio {
private:
    struct SoundInfo {
        std::string filePath;
        float volume = 0.1f;
        float pitch = 0.1f;
        bool loop = false;
    };

    std::unordered_map<int, SoundInfo> playingSounds;
    int nextSoundId;
    std::string currentBGM;
    float masterVolume;
    float bgmVolume;
    bool isBGMPlaying;
    bool isBGMInitialized;

public:
    SimpleAudio();
    ~SimpleAudio();

    // 初始
    bool Initialize();
    void Shutdown();

    // 背景音乐控制
    void PlayBGM(const std::string& filename, float volume = 1.0f, bool loop = true);
    void PauseBGM();
    void ResumeBGM();
    void StopBGM();
    void SetBGMVolume(float volume);

    // 音效播放
    int PlaySFX(const std::string& filename, float volume = 1.0f, bool loop = false);
    void StopSFX(int soundId);
    void PauseSFX(int soundId);
    void ResumeSFX(int soundId);
    void SetSFXVolume(int soundId, float volume);

    // 批量控制
    void PauseAll();
    void ResumeAll();
    void StopAll();
    void SetMasterVolume(float volume);

    void Update(float deltaTime);

    // 状态查询
    bool IsBGMPlaying() const;
    float GetBGMVolume() const;
    float GetMasterVolume() const;
};