// SimpleAudio.cpp
#include "SimpleAudio.h"

SimpleAudio::SimpleAudio()
    : nextSoundId(1)
    , masterVolume(1.0f)
    , bgmVolume(1.0f)
    , isBGMPlaying(false)
    , isBGMInitialized(false) {
}

SimpleAudio::~SimpleAudio() {
    Shutdown();
}

bool SimpleAudio::Initialize() {
    StopAll();
    isBGMInitialized = true;
    return true;
}

void SimpleAudio::Shutdown() {
    StopAll();
    playingSounds.clear();
    isBGMInitialized = false;
}

// 字符串转换辅助函数
std::wstring StringToWide(const std::string& str) {
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// 背景音乐控制
void SimpleAudio::PlayBGM(const std::string& filename, float volume, bool loop) {
    StopBGM();

    currentBGM = filename;
    bgmVolume = volume;
    isBGMPlaying = true;

    std::wstring wideFilename = StringToWide(filename);
    DWORD flags = SND_FILENAME | SND_ASYNC;
    if (loop) {
        flags |= SND_LOOP;
    }

    // 播放背景音乐
    PlaySoundW(wideFilename.c_str(), NULL, flags);
}

void SimpleAudio::PauseBGM() {
    if (isBGMPlaying) {
        PlaySoundW(NULL, NULL, 0);
        isBGMPlaying = false;
    }
}

void SimpleAudio::ResumeBGM() {
    if (!isBGMPlaying && !currentBGM.empty()) {
        std::wstring wideFilename = StringToWide(currentBGM);
        DWORD flags = SND_FILENAME | SND_ASYNC | SND_LOOP;
        PlaySoundW(wideFilename.c_str(), NULL, flags);
        isBGMPlaying = true;
    }
}

void SimpleAudio::StopBGM() {
    PlaySoundW(NULL, NULL, 0);
    currentBGM.clear();
    isBGMPlaying = false;
}

void SimpleAudio::SetBGMVolume(float volume) {
    bgmVolume = volume;
    // PlaySound不支持音量控制
}

// 音效播放
int SimpleAudio::PlaySFX(const std::string& filename, float volume, bool loop) {
    std::wstring wideFilename = StringToWide(filename);
    DWORD flags = SND_FILENAME | SND_ASYNC;
    if (loop) {
        flags |= SND_LOOP;
    }

    // 播放音效
    BOOL result = PlaySoundW(wideFilename.c_str(), NULL, flags);
    if (!result) {
        return -1;
    }

    int soundId = nextSoundId++;
    SoundInfo info;
    info.filePath = filename;
    info.volume = volume;
    info.pitch = 1.0f;
    info.loop = loop;

    playingSounds[soundId] = info;
    return soundId;
}

void SimpleAudio::StopSFX(int soundId) {
    // PlaySound没有停止特定音效的方法
    playingSounds.erase(soundId);

    // 如果没有音效在播放，停止所有声襾E
    if (playingSounds.empty()) {
        PlaySoundW(NULL, NULL, 0);
        // 恢复背景音乐
        if (isBGMPlaying && !currentBGM.empty()) {
            ResumeBGM();
        }
    }
}

void SimpleAudio::PauseSFX(int soundId) {
    // PlaySound不支持暂停特定音效
    playingSounds.erase(soundId);
}

void SimpleAudio::ResumeSFX(int soundId) {
    // PlaySound不支持恢复特定音效
    auto it = playingSounds.find(soundId);
    if (it != playingSounds.end()) {
        PlaySFX(it->second.filePath, it->second.volume, it->second.loop);
    }
}

void SimpleAudio::SetSFXVolume(int soundId, float volume) {
    auto it = playingSounds.find(soundId);
    if (it != playingSounds.end()) {
        it->second.volume = volume;
    }
}

// 批量控制
void SimpleAudio::PauseAll() {
    PlaySoundW(NULL, NULL, 0);
    isBGMPlaying = false;
}

void SimpleAudio::ResumeAll() {
    if (!currentBGM.empty()) {
        ResumeBGM();
    }
}

void SimpleAudio::StopAll() {
    PlaySoundW(NULL, NULL, 0);
    playingSounds.clear();
    currentBGM.clear();
    isBGMPlaying = false;
}

void SimpleAudio::SetMasterVolume(float volume) {
    masterVolume = volume;
    // PlaySound不支持音量控制
}

// 竵E?
void SimpleAudio::Update(float deltaTime) {
    // PlaySound没有提供查询状态的方法
}

// 状态查询
bool SimpleAudio::IsBGMPlaying() const {
    return isBGMPlaying;
}

float SimpleAudio::GetBGMVolume() const {
    return bgmVolume;
}

float SimpleAudio::GetMasterVolume() const {
    return masterVolume;
}