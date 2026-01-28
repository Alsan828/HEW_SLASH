#include "Audio.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <xaudio2.h> // this is better bc its real time sound and it wil be instantly. it can play more than one sound at once
#include <vector>
#include <unordered_map>
#include <algorithm>

#pragma comment(lib, "xaudio2.lib")

namespace
{
	float Clamp01(float v)
	{
		if (v < 0.0f) return 0.0f;
		if (v > 1.0f) return 1.0f;
		return v;
	}

	// XAudio2 objects
	IXAudio2* g_xaudio2 = nullptr;
	IXAudio2MasteringVoice* g_masterVoice = nullptr;

	// Volume settings
	float g_master = 1.0f;
	float g_bgm = 1.0f;
	float g_se = 1.0f;

	// BGM voice and data
	IXAudio2SourceVoice* g_bgmVoice = nullptr;
	std::vector<BYTE> g_bgmData;

	bool g_inited = false;

	// Sound effect data
	struct SoundData
	{
		std::vector<BYTE> audioData;
		WAVEFORMATEX waveFormat;
	};

	// Cache for preloaded sound effects
	std::unordered_map<std::string, SoundData> g_soundCache;

	// so it tracks the current bgm
	std::string g_currentBGM = "";

	// Helper function to load WAV file
	bool LoadWavFile(const std::string& filePath, std::vector<BYTE>& audioData, WAVEFORMATEX& waveFormat)
	{
		// Open the file
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, 0, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		// Read the file into memory
		DWORD fileSize = GetFileSize(hFile, nullptr);
		std::vector<BYTE> fileData(fileSize);
		DWORD bytesRead;
		if (!ReadFile(hFile, fileData.data(), fileSize, &bytesRead, nullptr))
		{
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);

		// Parse WAV file (simple parser for PCM WAV files)
		if (fileSize < 44) return false;

		// Check RIFF header
		if (memcmp(fileData.data(), "RIFF", 4) != 0) return false;
		if (memcmp(fileData.data() + 8, "WAVE", 4) != 0) return false;

		// Find fmt chunk
		size_t offset = 12;
		while (offset + 8 <= fileSize)
		{
			if (memcmp(fileData.data() + offset, "fmt ", 4) == 0)
			{
				DWORD fmtSize = *(DWORD*)(fileData.data() + offset + 4);
				if (offset + 8 + fmtSize > fileSize) return false;

				// Copy wave format
				size_t copySize = std::min((size_t)sizeof(WAVEFORMATEX), (size_t)fmtSize);
				memcpy(&waveFormat, fileData.data() + offset + 8, copySize);
				offset += 8 + fmtSize;
				break;
			}
			else
			{
				DWORD chunkSize = *(DWORD*)(fileData.data() + offset + 4);
				offset += 8 + chunkSize;
			}
		}

		// Find data chunk
		while (offset + 8 <= fileSize)
		{
			if (memcmp(fileData.data() + offset, "data", 4) == 0)
			{
				DWORD dataSize = *(DWORD*)(fileData.data() + offset + 4);
				if (offset + 8 + dataSize > fileSize) return false;

				// Copy audio data
				audioData.resize(dataSize);
				memcpy(audioData.data(), fileData.data() + offset + 8, dataSize);
				return true;
			}
			else
			{
				DWORD chunkSize = *(DWORD*)(fileData.data() + offset + 4);
				offset += 8 + chunkSize;
			}
		}

		return false;
	}
}

namespace Audio
{
	bool Init()
	{
		if (g_inited) return true;

		// Initialize COM (because of XAudio2)
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		// Create XAudio2 engine
		HRESULT hr = XAudio2Create(&g_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(hr))
		{
			CoUninitialize();
			return false;
		}

		// Create mastering voice
		hr = g_xaudio2->CreateMasteringVoice(&g_masterVoice);
		if (FAILED(hr))
		{
			g_xaudio2->Release();
			g_xaudio2 = nullptr;
			CoUninitialize();
			return false;
		}

		g_inited = true;
		return true;
	}

	void Shutdown()
	{
		if (!g_inited) return;

		StopBGM();
		g_soundCache.clear();

		if (g_masterVoice)
		{
			g_masterVoice->DestroyVoice();
			g_masterVoice = nullptr;
		}

		if (g_xaudio2)
		{
			g_xaudio2->Release();
			g_xaudio2 = nullptr;
		}

		CoUninitialize();
		g_inited = false;
	}

	void SetMasterVolume(float volume01)
	{
		g_master = Clamp01(volume01);
		if (g_masterVoice)
		{
			g_masterVoice->SetVolume(g_master);
		}
	}

	void SetBgmVolume(float volume01)
	{
		g_bgm = Clamp01(volume01);
		if (g_bgmVoice)
		{
			g_bgmVoice->SetVolume(g_bgm);
		}
	}

	void SetSeVolume(float volume01)
	{
		g_se = Clamp01(volume01);
	}

	void PlayBGM(const std::string& filePath, bool loop)
	{
		if (!g_inited) Init();

		// dont restart the bgm is its playing the current bgm I want
		if (g_currentBGM == filePath && g_bgmVoice != nullptr)
			return;

		StopBGM();

		g_currentBGM = filePath;

		// Load the BGM file
		WAVEFORMATEX waveFormat = {};
		if (!LoadWavFile(filePath, g_bgmData, waveFormat))
			return;

		// Create source voice
		HRESULT hr = g_xaudio2->CreateSourceVoice(&g_bgmVoice, &waveFormat);
		if (FAILED(hr))
			return;

		// Set up the audio buffer
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = (UINT32)g_bgmData.size();
		buffer.pAudioData = g_bgmData.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		if (loop)
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

		// Submit and play
		g_bgmVoice->SubmitSourceBuffer(&buffer);
		g_bgmVoice->SetVolume(g_bgm);
		g_bgmVoice->Start(0);
	}

	void StopBGM()
	{
		if (g_bgmVoice)
		{
			g_bgmVoice->Stop(0);
			g_bgmVoice->DestroyVoice();
			g_bgmVoice = nullptr;
		}
		g_bgmData.clear();
		//g_currentBGM = "";
	}

	void PauseBGM()
	{
		if (g_bgmVoice)
		{
			g_bgmVoice->Stop(0);
		}
	}

	void ResumeBGM()
	{
		if (g_bgmVoice)
		{
			g_bgmVoice->Start(0);
		}
	}

	// use this we we make sure the SE will play instantly and there will not be any delay
	void PreloadSE(const std::string& filePath)
	{
		if (!g_inited) Init();

		// Check if already loaded
		if (g_soundCache.find(filePath) != g_soundCache.end())
			return;

		// Load the sound effect
		SoundData soundData;
		if (LoadWavFile(filePath, soundData.audioData, soundData.waveFormat))
		{
			g_soundCache[filePath] = std::move(soundData);
		}
	}

	void PlaySE(const std::string& filePath, float volume01)
	{
		if (!g_inited) Init();

		// Preload if not cached
		if (g_soundCache.find(filePath) == g_soundCache.end())
		{
			PreloadSE(filePath);
			if (g_soundCache.find(filePath) == g_soundCache.end())
				return; // Failed to load
		}

		const SoundData& soundData = g_soundCache[filePath];

		// Create a source voice for this sound effect
		IXAudio2SourceVoice* sourceVoice = nullptr;
		HRESULT hr = g_xaudio2->CreateSourceVoice(&sourceVoice, &soundData.waveFormat);
		if (FAILED(hr))
			return;

		// Set up the audio buffer
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = (UINT32)soundData.audioData.size();
		buffer.pAudioData = soundData.audioData.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		// Calculate final volume
		float finalVolume = g_se * Clamp01(volume01);
		sourceVoice->SetVolume(finalVolume);

		// Submit and play
		sourceVoice->SubmitSourceBuffer(&buffer);
		sourceVoice->Start(0);
	}

	// for getting the current bgm
	std::string GetCurrentBGMPath()
	{
		return g_currentBGM;
	}

	// for checkiing if the bgm is playing or not
	bool IsBGMPlaying()
	{
		return (g_bgmVoice != nullptr);
	}
}

