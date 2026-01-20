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
	void StopBGM();

	void PlaySE(const std::string& filePath, float volume01 = 1.0f);
}
