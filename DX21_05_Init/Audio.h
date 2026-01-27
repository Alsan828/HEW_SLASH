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

	void PreloadSE(const std::string& filePath); // use this we we make sure the SE will play instantly and there will not be any delay

}
