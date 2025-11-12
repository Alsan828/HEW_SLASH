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

// Window and screen settings
#define CLASS_NAME   "DX21Smpl"
#define WINDOW_NAME  "SLASH"
#define SCREEN_WIDTH (1080)
#define SCREEN_HEIGHT (680)

struct VertexV {
	float x, y, z;
	float u, v;
	float r, g, b, a; // for the color
};

struct ConstantBuffer 
{
	DirectX::XMMATRIX worldView;
	DirectX::XMMATRIX projection;

	// added november 12th
	DirectX::XMFLOAT4 color; // for the color
	DirectX::XMMATRIX matrixTex;   	// for the UV //send it to the const buffer
};

// Use extern to declare global variables (without initialization)
extern ID3D11Device* g_pDevice;            // Device object
extern ID3D11DeviceContext* g_pDeviceContext; // Device context

extern ID3D11Buffer* g_pConstantBuffer; // added november 12th

extern ID3D11InputLayout* g_pInputLayout;    // Input layout
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

// Function declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT RendererInit(HWND hwnd);
void RendererDrawF();// Called before rendering
void RendererDrawB();// Called after rendering
void RendererUninit();
HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName);
HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ppShaderObject, int* pShaderObjectSize);
HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName);
void RenderQuad(const VertexV vertices[4], ID3D11VertexShader* pVS, ID3D11PixelShader* pPS);
void RenderNumber(int number, float startX, float startY, float digitWidth, float digitHeight, ID3D11ShaderResourceView* textureSRV);
void RenderImage(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV, int frameIndex, int rows, int columns);

void SetColor(float r, float g, float b, float a); // added november 12th