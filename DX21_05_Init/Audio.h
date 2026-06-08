#pragma once
#include <string>

namespace Audio
{
	bool Init();
	void Shutdown();
	void SetMasterVolume(float volume01);
	void SetBgmVolume(float volume01);
	void SetSeVolume(float volume01);
	void PlayBGM(const std::string& filePath, bool loop = true);
	void PauseBGM();
	void StopBGM();
	void ResumeBGM();
	void PlaySE(const std::string& filePath, float volume01 = 1.0f);

	void PreloadSE(const std::string& filePath); // SE を事前に読み込み、再生遅延を防ぐ

	std::string GetCurrentBGMPath();
	bool IsBGMPlaying();
}
