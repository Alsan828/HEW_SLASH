#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <windowsx.h>
#include "Texture1.h"
#include "stb_image.h"
#include <locale.h>
#include <atltypes.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <DirectXMath.h>

// ウィンドウと画面設定
#define CLASS_NAME   "DX21Smpl"
#define WINDOW_NAME  "SLASH"
#define SCREEN_WIDTH (1080)
#define SCREEN_HEIGHT (680)

struct VertexV {
	float x, y, z;
	float u, v;
	float r, g, b, a; // 色用
};

constexpr int MAX_LINEAR_CLIP_PLANES = 16;

struct LinearClipPlane
{
	float normalX;
	float normalY;
	float centerU;
	float centerV;
	float keepSide;
};

struct ConstantBuffer 
{
	DirectX::XMMATRIX worldView;
	DirectX::XMMATRIX projection;

	// 11 月 12 日追加
	DirectX::XMFLOAT4 color; // 色用
	DirectX::XMMATRIX matrixTex;   	// UV 用。定数バッファへ送る

	float fillRatio; // ゲージ充填率
	float useGaugeFill; // ゲージ充填モード用
	float useLinearClip; // 任意の半平面クリップを有効にする
	float clipPlaneCount; // 使用中のクリップ平面数
	DirectX::XMFLOAT4 clipPlanes[MAX_LINEAR_CLIP_PLANES]; // normal.x, normal.y, centerU, centerV

};

// extern を使ってグローバル変数を宣言する（初期化はしない）
extern ID3D11Device* g_pDevice;            // デバイスオブジェクト
extern ID3D11DeviceContext* g_pDeviceContext; // デバイスコンテキスト

extern ID3D11Buffer* g_pConstantBuffer; // 11 月 12 日追加

extern ID3D11InputLayout* g_pInputLayout;    // 入力レイアウト
extern ID3D11ShaderResourceView* pTextureSRV;
extern ID3D11ShaderResourceView* pTextureSRV2;
extern ID3D11ShaderResourceView* pTextureSRV3;
extern ID3D11ShaderResourceView* pTextureNum;
extern D3D_FEATURE_LEVEL m_FeatureLevel;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;
extern ID3D11DepthStencilView* g_pDepthStencilView;
extern ID3D11Buffer* g_pVertexBuffer;
extern ID3D11VertexShader* g_pVertexShader;
extern ID3D11PixelShader* g_pPixelShader;

extern D3D11_SAMPLER_DESC sampDesc;
extern ID3D11SamplerState* pSamplerState;

// 関数宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT RendererInit(HWND hwnd);
void RendererDrawF();// 描画前に呼ぶ
void RendererDrawB();// 描画後に呼ぶ
void RendererUninit();
HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName);
HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ppShaderObject, int* pShaderObjectSize);
HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName);
void RenderQuad(const VertexV vertices[4], ID3D11VertexShader* pVS, ID3D11PixelShader* pPS);
void RenderNumber(int number, float startX, float startY, float digitWidth, float digitHeight, ID3D11ShaderResourceView* textureSRV, bool enableCulling = true);
void RenderImage(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV,
	int frameIndex = 0, int rows = 1, int columns = 1, bool enableCulling = false,
	float rotation = 0.0f, bool flipHorizontal = false); // 回転対応を追加

void RenderImageClipped(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV, float texClipRight);

void RenderGaugeFillImage(float posX, float posY, float width, float height,
	ID3D11ShaderResourceView* textureSRV, float fillRatio); // ダイヤ形のゲージバー描画用

void SetLinearClipPlanes(const LinearClipPlane* planes, int count);
void SetLinearClip(bool enabled, float normalX = 0.0f, float normalY = 0.0f,
	float centerU = 0.5f, float centerV = 0.5f, float keepSide = 1.0f);

void SetColor(float r, float g, float b, float a); // 11 月 12 日追加

// シーン描画後、Present() 前に呼ばれる。
// 常時表示オーバーレイ要素（例: グローバルなゲーム内カーソル）向け。
void RenderOverlay();
