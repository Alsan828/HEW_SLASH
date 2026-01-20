#include "Audio.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

namespace
{
	float Clamp01(float v)
	{
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	DWORD Volume01ToWinmm(float v01)
	{
		v01 = Clamp01(v01);
		DWORD v = static_cast<DWORD>(v01 * 0xFFFF);
		return (v & 0xFFFF) | (v << 16);
	}

	float g_master = 1.0f;
	float g_bgm = 1.0f;
	float g_se = 1.0f;

	std::string g_bgmAlias = "BGM";
	bool g_inited = false;
}

namespace Audio
{
	bool Init()
	{
		g_inited = true;
		return true;
	}

	void Shutdown()
	{
		StopBGM();
		g_inited = false;
	}

	void SetMasterVolume(float volume01)
	{
		g_master = Clamp01(volume01);
		// WinMM global waveOut volume affects all waveOut output.
		waveOutSetVolume(nullptr, Volume01ToWinmm(g_master));
	}

	void SetBgmVolume(float volume01)
	{
		g_bgm = Clamp01(volume01);
		// MCI per-alias volume is not consistently supported for WAV.
		// Keep stored value in case you later switch to a different backend.
	}

	void SetSeVolume(float volume01)
	{
		g_se = Clamp01(volume01);
	}

	void PlayBGM(const std::string& filePath, bool loop)
	{
		if (!g_inited) Init();

		StopBGM();

		std::string openCmd = "open \"" + filePath + "\" type waveaudio alias " + g_bgmAlias;
		mciSendStringA(openCmd.c_str(), nullptr, 0, nullptr);

		std::string playCmd = "play " + g_bgmAlias;
		if (loop)
			playCmd += " repeat";
		mciSendStringA(playCmd.c_str(), nullptr, 0, nullptr);
	}

	void StopBGM()
	{
		if (!g_inited)	return;
		mciSendStringA(("stop " + g_bgmAlias).c_str(), nullptr, 0, nullptr);
		mciSendStringA(("close " + g_bgmAlias).c_str(), nullptr, 0, nullptr);
	}

	void PlaySE(const std::string& filePath, float volume01)
	{
		if (!g_inited) Init();

		// WinMM doesn't provide per-sound volume for PlaySound; approximate by scaling global volume.
		float oldMaster = g_master;
		float newMaster = Clamp01(oldMaster * Clamp01(volume01) * g_se);
		waveOutSetVolume(nullptr, Volume01ToWinmm(newMaster));

		PlaySoundA(filePath.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);

		waveOutSetVolume(nullptr, Volume01ToWinmm(oldMaster));
	}
}
