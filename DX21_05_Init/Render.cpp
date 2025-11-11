#include "Render.h"
#include "Game.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }

// Global variable definitions (allow initialization)
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11ShaderResourceView* pTextureSRV = nullptr;
ID3D11ShaderResourceView* pTextureSRV2 = nullptr;
ID3D11ShaderResourceView* pTextureSRV3 = nullptr;
ID3D11ShaderResourceView* pTextureNum = nullptr;
D3D11_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11BlendState* g_pBlendState = nullptr;

ID3D11ShaderResourceView* texIdle1 = nullptr;
ID3D11ShaderResourceView* texIdle2 = nullptr;
ID3D11ShaderResourceView* texIdle3 = nullptr;
ID3D11ShaderResourceView* texRun1 = nullptr;
ID3D11ShaderResourceView* texRun2 = nullptr;
ID3D11ShaderResourceView* texRun3 = nullptr;

D3D11_SAMPLER_DESC sampDesc = {};
ID3D11SamplerState* pSamplerState = nullptr;

HRESULT RendererInit(HWND hwnd) {
	HRESULT hr = S_OK;

	// Get actual window client area size
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	UINT windowWidth = clientRect.right - clientRect.left;
	UINT windowHeight = clientRect.bottom - clientRect.top;

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = windowWidth;      // Use actual window width
	swapChainDesc.BufferDesc.Height = windowHeight;    // Use actual window height
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;  // Maintain windowed mode (borderless fullscreen)

	// Call function to create device and swap chain simultaneously
	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&g_pSwapChain,
		&g_pDevice,
		&m_FeatureLevel,
		&g_pDeviceContext);
	if (FAILED(hr)) return hr;

	// Create render target view
	ID3D11Texture2D* renderTarget;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = g_pDevice->CreateRenderTargetView(renderTarget, NULL, &g_pRenderTargetView);
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	// Create depth stencil buffer
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = windowWidth;   // Use actual window width
	textureDesc.Height = windowHeight; // Use actual window height
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	hr = g_pDevice->CreateTexture2D(&textureDesc, NULL, &depthStencile);
	if (FAILED(hr)) return hr;

	// Create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &g_pDepthStencilView);
	if (FAILED(hr)) return hr;
	depthStencile->Release();

	// Create viewport (using actual window dimensions)
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)windowWidth;
	viewport.Height = (FLOAT)windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	g_pDeviceContext->RSSetViewports(1, &viewport);

	// Create input layout
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		// Indicate presence of position coordinates
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// Indicate presence of texture coordinates
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// Create vertex shader object and vertex layout simultaneously
	hr = CreateVertexShader(&g_pVertexShader, &g_pInputLayout, layout, numElements, "VertexShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return hr;
	}

	// Create pixel shader object
	hr = CreatePixelShader(&g_pPixelShader, "PixelShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/char02.png", &pTextureSRV);
	if (FAILED(hr)) {
		// Handle error
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}
	hr = LoadTexture(g_pDevice, "asset/back_img_01.png", &pTextureSRV2);
	if (FAILED(hr)) {
		// Handle error
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/char01.png", &pTextureSRV3);
	if (FAILED(hr)) {
		// Handle error
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}
	hr = LoadTexture(g_pDevice, "asset/Number.png", &pTextureNum);
	if (FAILED(hr)) {
		// Handle error
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	// Create sampler state
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_pDevice->CreateSamplerState(&sampDesc, &pSamplerState);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE; // Disable alpha to coverage
	blendDesc.IndependentBlendEnable = FALSE; // Disable independent blending
	blendDesc.RenderTarget[0].BlendEnable = TRUE; // Enable blending for render target
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Source blend factor
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // Destination blend factor
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // Blend operation
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha source blend factor
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha destination blend factor
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha blend operation
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // Write to all color channels
	hr = g_pDevice->CreateBlendState(&blendDesc, &g_pBlendState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBlendState error", "error", MB_OK);
		return hr;
	}

	ID3D11DepthStencilState* depthStencilState = nullptr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // Depth write mask
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // Depth comparison function
	hr = g_pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateDepthStencilState error", "error", MB_OK);
		return hr;
	}
	g_pDeviceContext->OMSetDepthStencilState(depthStencilState, 1); // Set depth stencil state

	return S_OK;
}

void RendererDrawF()
{
	// Screen fill color
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //red,green,blue,alpha

	// Specify rendering target canvas and depth buffer to use
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	// Fill the rendering target canvas
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	// Reset depth buffer
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT strides = sizeof(VertexV);
	UINT offsets = 0;
	g_pDeviceContext->IASetInputLayout(g_pInputLayout);
	g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &strides, &offsets);
	g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);

	g_pDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF); // Set blend state (use default blend state)

	// Background: pTextureSRV2
	RenderImage(-1.f, -1.f, 4.75f, 4.75f, pTextureSRV2, 0, 1, 1);
	// Set sampler state
}

void RendererDrawB()
{
	// Switch double buffer and update screen
	g_pSwapChain->Present(0, 0);
}

void RendererUninit()
{
	// ※All DirectX features must be released when the application terminates after creation
	if (g_pDeviceContext) g_pDeviceContext->ClearState();
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVertexShader);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pDepthStencilView);
	SAFE_RELEASE(g_pRenderTargetView);
	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);
	SAFE_RELEASE(pTextureSRV);
	SAFE_RELEASE(pTextureSRV2);
}

HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ppShaderObject, int* pShaderObjectSize)
{
	// Create .cso filename
	char csoFileName[256];
	const char* dot = strrchr(szFileName, '.');  // Find last '.'
	if (dot) {
		int basenameLen = dot - szFileName;
		strncpy(csoFileName, szFileName, basenameLen); // Copy filename without extension if extension exists
		csoFileName[basenameLen] = '\0';   // Add null terminator
	}
	else {
		strcpy(csoFileName, szFileName);   // Copy as is if no extension
	}
	strcat(csoFileName, ".cso");// Add ".cso" extension

	// Open .cso file if it exists
	FILE* fp;
	int ret = fopen_s(&fp, csoFileName, "rb");
	if (ret == 0)
	{
		// Get file size
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// Allocate memory for binary data reading
		unsigned char* byteArray = new unsigned char[size];
		fread(byteArray, size, 1, fp);
		fclose(fp);

		// Pass pointer and size to caller
		*ppShaderObject = byteArray;
		*pShaderObjectSize = size;
	}
	// If .cso file doesn't exist, compile .hlsl file
	else
	{
		HRESULT hr = S_OK;
		WCHAR	filename[512];
		size_t 	wLen = 0;
		int err = 0;

		// Convert character encoding from Shift-JIS to UTF-16
		setlocale(LC_ALL, "japanese");  // Set locale (Windows specific)
		err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

		// Set shader compile options
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG; // Include debug information for debug builds
#endif

		// Blobs for storing compile results and error information
		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlob = nullptr;

		// Compile HLSL file
		hr = D3DCompileFromFile(
			filename,							// Filename
			nullptr,							// No macro definitions 
			D3D_COMPILE_STANDARD_FILE_INCLUDE,	// #include support 
			szEntryPoint,						// Entry point name
			szShaderModel,						// Shader model
			dwShaderFlags,						// Compile flags
			0,									// Effect flags
			&pBlob,								// Compile result on success
			&pErrorBlob);						// Compile error output

		// Display error message on compile failure
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr) {
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			if (pBlob)(pBlob)->Release();
			return E_FAIL;
		}

		// Release error blob if it exists
		if (pErrorBlob) pErrorBlob->Release();

		// Pass compiled binary data to caller on success
		*ppShaderObject = (pBlob)->GetBufferPointer();
		*pShaderObjectSize = (pBlob)->GetBufferSize();
	}

	return S_OK;
}

HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// Compile according to file extension
	HRESULT hr = CompileShader(szFileName, "main", "vs_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return E_FAIL;

	// Create vertex shader using device
	hr = g_pDevice->CreateVertexShader(ShaderObject, ShaderObjectSize, NULL, ppVertexShader);

	// Create vertex layout using device
	g_pDevice->CreateInputLayout(pLayout, numElements, ShaderObject, ShaderObjectSize, ppVertexLayout);

	return S_OK;
}

HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// Compile according to file extension
	HRESULT hr = CompileShader(szFileName, "main", "ps_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return hr;

	// Create pixel shader
	hr = g_pDevice->CreatePixelShader(ShaderObject, ShaderObjectSize, nullptr, ppPixelShader);
	if (FAILED(hr)) return hr;

	return S_OK;
}

// Quad rendering function
void RenderQuad(const VertexV vertices[4], ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
	// Create temporary vertex buffer
	ID3D11Buffer* pQuadBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(VertexV) * 4;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices;
	g_pDevice->CreateBuffer(&bufferDesc, &initData, &pQuadBuffer);

	// Set render pipeline state
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	// Execute draw command
	g_pDeviceContext->Draw(4, 0);

	// Release temporary resources
	SAFE_RELEASE(pQuadBuffer);
}

void RenderImage(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV, int frameIndex = 0, int rows = 1, int columns = 1)
{
	// Calculate size of each frame in sprite sheet (texture coordinates)
	float frameWidth = 1.0f / columns;
	float frameHeight = 1.0f / rows;

	// Calculate current frame position in sprite sheet
	int row = frameIndex / columns;
	int col = frameIndex % columns;

	// Calculate texture coordinate range for current frame
	float u0 = col * frameWidth;       // Left boundary
	float u1 = (col + 1) * frameWidth; // Right boundary
	float v0 = row * frameHeight;      // Top boundary
	float v1 = (row + 1) * frameHeight;// Bottom boundary

	// Create vertex data (with correct texture coordinates)
	VertexV vertices[4] = {
		// Position coordinates                   // Texture coordinates
		{ posX + width, posY + height, 0.5f, u1, v0 }, // Top right
		{ posX + width, posY,          0.5f, u1, v1 }, // Bottom right
		{ posX,         posY + height, 0.5f, u0, v0 }, // Top left
		{ posX,         posY,          0.5f, u0, v1 }  // Bottom left
	};

	// Create temporary vertex buffer
	ID3D11Buffer* pDynamicBuffer = nullptr;
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(vertices);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	HRESULT hr = g_pDevice->CreateBuffer(&desc, &initData, &pDynamicBuffer);
	if (FAILED(hr)) {
		// Error handling
		return;
	}

	// Bind vertex buffer
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &pDynamicBuffer, &stride, &offset);

	// Bind texture resource
	g_pDeviceContext->PSSetShaderResources(0, 1, &textureSRV);

	// Set sampler state
	g_pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

	g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF); // Set blend state

	// Draw quad
	g_pDeviceContext->Draw(4, 0);

	// Release temporary resources
	SAFE_RELEASE(pDynamicBuffer);
}

void RenderNumber(int number, float startX, float startY, float digitWidth, float digitHeight, ID3D11ShaderResourceView* textureSRV) {
	if (number < 0) return; // Only support non-negative numbers

	// Calculate total number of digits (e.g., 123 has 3 digits)
	int numDigits = 0;
	int temp = number;
	while (temp > 0) {
		temp /= 10;
		numDigits++;
	}
	if (numDigits == 0) numDigits = 1; // Handle case of 0

	// Render digits from right to left (right-aligned)
	float currentX = startX + (numDigits - 1) * digitWidth; // Right alignment starting point
	temp = number;
	for (int i = 0; i < numDigits; i++) {
		int digit = temp % 10; // Get current digit (ones place)
		temp /= 10;            // Remove processed digit

		// Render current digit
		RenderImage(currentX, startY, digitWidth, digitHeight, textureSRV, digit, 1, 10);
		currentX -= digitWidth; // Move left one digit
	}
}