#include "Audio.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <xaudio2.h> // リアルタイム再生に向いており、反応が速く複数の音を同時再生できる
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

	// XAudio2 オブジェクト
	IXAudio2* g_xaudio2 = nullptr;
	IXAudio2MasteringVoice* g_masterVoice = nullptr;

	// 音量設定
	float g_master = 1.0f;
	float g_bgm = 1.0f;
	float g_se = 1.0f;

	// BGM のボイスとデータ
	IXAudio2SourceVoice* g_bgmVoice = nullptr;
	std::vector<BYTE> g_bgmData;

	bool g_inited = false;

	// 効果音データ
	struct SoundData
	{
		std::vector<BYTE> audioData;
		WAVEFORMATEX waveFormat;
	};

	// 事前読み込みした効果音のキャッシュ
	std::unordered_map<std::string, SoundData> g_soundCache;

	// 現在の BGM を追跡する
	std::string g_currentBGM = "";

	// WAV ファイルを読み込む補助関数
	bool LoadWavFile(const std::string& filePath, std::vector<BYTE>& audioData, WAVEFORMATEX& waveFormat)
	{
		// ファイルを開く
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
			nullptr, OPEN_EXISTING, 0, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;

		// ファイルをメモリへ読み込む
		DWORD fileSize = GetFileSize(hFile, nullptr);
		std::vector<BYTE> fileData(fileSize);
		DWORD bytesRead;
		if (!ReadFile(hFile, fileData.data(), fileSize, &bytesRead, nullptr))
		{
			CloseHandle(hFile);
			return false;
		}
		CloseHandle(hFile);

		// WAV ファイルを解析する（PCM WAV 用の簡易パーサー）
		if (fileSize < 44) return false;

		// RIFF ヘッダーを確認
		if (memcmp(fileData.data(), "RIFF", 4) != 0) return false;
		if (memcmp(fileData.data() + 8, "WAVE", 4) != 0) return false;

		// fmt チャンクを探す
		size_t offset = 12;
		while (offset + 8 <= fileSize)
		{
			if (memcmp(fileData.data() + offset, "fmt ", 4) == 0)
			{
				DWORD fmtSize = *(DWORD*)(fileData.data() + offset + 4);
				if (offset + 8 + fmtSize > fileSize) return false;

				// waveFormat をコピーする
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

		// data チャンクを探す
		while (offset + 8 <= fileSize)
		{
			if (memcmp(fileData.data() + offset, "data", 4) == 0)
			{
				DWORD dataSize = *(DWORD*)(fileData.data() + offset + 4);
				if (offset + 8 + dataSize > fileSize) return false;

				// 音声データをコピーする
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

		// COM を初期化する（XAudio2 のため）
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		// XAudio2 エンジンを生成する
		HRESULT hr = XAudio2Create(&g_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (FAILED(hr))
		{
			CoUninitialize();
			return false;
		}

		// マスタリングボイスを生成する
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

		// 同じ BGM が再生中なら再起動しない
		if (g_currentBGM == filePath && g_bgmVoice != nullptr)
			return;

		StopBGM();

		g_currentBGM = filePath;

		// BGM ファイルを読み込む
		WAVEFORMATEX waveFormat = {};
		if (!LoadWavFile(filePath, g_bgmData, waveFormat))
			return;

		// ソースボイスを生成する
		HRESULT hr = g_xaudio2->CreateSourceVoice(&g_bgmVoice, &waveFormat);
		if (FAILED(hr))
			return;

		// オーディオバッファを設定する
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = (UINT32)g_bgmData.size();
		buffer.pAudioData = g_bgmData.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		if (loop)
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

		// バッファを送信して再生する
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
		// g_currentBGM = "";
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

	// SE を事前に読み込み、再生遅延を防ぐ
	void PreloadSE(const std::string& filePath)
	{
		if (!g_inited) Init();

		// すでに読み込み済みか確認する
		if (g_soundCache.find(filePath) != g_soundCache.end())
			return;

		// 効果音を読み込む
		SoundData soundData;
		if (LoadWavFile(filePath, soundData.audioData, soundData.waveFormat))
		{
			g_soundCache[filePath] = std::move(soundData);
		}
	}

	void PlaySE(const std::string& filePath, float volume01)
	{
		if (!g_inited) Init();

		// キャッシュされていなければ事前読み込みする
		if (g_soundCache.find(filePath) == g_soundCache.end())
		{
			PreloadSE(filePath);
			if (g_soundCache.find(filePath) == g_soundCache.end())
				return; // 読み込み失敗
		}

		const SoundData& soundData = g_soundCache[filePath];

		// この効果音用のソースボイスを生成する
		IXAudio2SourceVoice* sourceVoice = nullptr;
		HRESULT hr = g_xaudio2->CreateSourceVoice(&sourceVoice, &soundData.waveFormat);
		if (FAILED(hr))
			return;

		// オーディオバッファを設定する
		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = (UINT32)soundData.audioData.size();
		buffer.pAudioData = soundData.audioData.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		// 最終音量を計算する
		float finalVolume = g_se * Clamp01(volume01);
		sourceVoice->SetVolume(finalVolume);

		// バッファを送信して再生する
		sourceVoice->SubmitSourceBuffer(&buffer);
		sourceVoice->Start(0);
	}

	// 現在の BGM パスを取得する
	std::string GetCurrentBGMPath()
	{
		return g_currentBGM;
	}

	// BGM が再生中かどうかを確認する
	bool IsBGMPlaying()
	{
		return (g_bgmVoice != nullptr);
	}
}

