#include "Render.h"
#include "Game.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }


// 定义全局变量（允许初始化）
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11ShaderResourceView* pTextureSRV = nullptr;
ID3D11ShaderResourceView* pTextureSRV2 = nullptr;
ID3D11ShaderResourceView* pTextureSRV3 = nullptr;
ID3D11ShaderResourceView* pTextureNum = nullptr;
D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
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

	// 获取窗口客户区实际大小
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	UINT windowWidth = clientRect.right - clientRect.left;
	UINT windowHeight = clientRect.bottom - clientRect.top;

	// デバイス、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = windowWidth;      // 使用实际窗口宽度
	swapChainDesc.BufferDesc.Height = windowHeight;    // 使用实际窗口高度
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;  // 保持为窗口模式（无边框全屏）

	// デバイスとスワップチェインを同時に作成する関数の呼び出し
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

	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = g_pDevice->CreateRenderTargetView(renderTarget, NULL, &g_pRenderTargetView);
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	// デプスステンシルバッファ作成
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = windowWidth;   // 使用实际窗口宽度
	textureDesc.Height = windowHeight; // 使用实际窗口高度
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

	// デプスステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &g_pDepthStencilView);
	if (FAILED(hr)) return hr;
	depthStencile->Release();

	// ビューポートを作成（使用实际窗口尺寸）
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)windowWidth;
	viewport.Height = (FLOAT)windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	g_pDeviceContext->RSSetViewports(1, &viewport);
	// インプットレイアウト作成
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		// 位置座標があるということを伝える
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// テクスチャ座標があるということを伝える
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	hr = CreateVertexShader(&g_pVertexShader, &g_pInputLayout, layout, numElements, "VertexShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return hr;
	}

	// ピクセルシェーダーオブジェクトを生成
	hr = CreatePixelShader(&g_pPixelShader, "PixelShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/char02.png", &pTextureSRV);
	if (FAILED(hr)) {
		// 处理错误		
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}
	hr = LoadTexture(g_pDevice, "asset/back_img_01.png", &pTextureSRV2);
	if (FAILED(hr)) {
		// 处理错误		
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/char01.png", &pTextureSRV3);
	if (FAILED(hr)) {
		// 处理错误		
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}
	hr = LoadTexture(g_pDevice, "asset/Number.png", &pTextureNum);
	if (FAILED(hr)) {
		// 处理错误		
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	// 创建采样器状态
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_pDevice->CreateSamplerState(&sampDesc, &pSamplerState);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE; // 不启用Alpha到覆盖
	blendDesc.IndependentBlendEnable = FALSE; // 不启用独立混合
	blendDesc.RenderTarget[0].BlendEnable = TRUE; // 启用渲染目标的混合
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 源混合因子
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 目标混合因子
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // 混合操作
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha源混合因子
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha目标混合因子
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha混合操作
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 写入所有颜色通道
	hr = g_pDevice->CreateBlendState(&blendDesc, &g_pBlendState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBlendState error", "error", MB_OK);
		return hr;
	}

	ID3D11DepthStencilState* depthStencilState = nullptr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 深度写入掩码
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // 深度比较函数
	hr = g_pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateDepthStencilState error", "error", MB_OK);
		return hr;
	}
	g_pDeviceContext->OMSetDepthStencilState(depthStencilState, 1); // 设置深度模板状态

	return S_OK;
}


void RendererDrawF()
{
	// 画面塗りつぶし色
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //red,green,blue,alpha

	// 描画先のキャンバスと使用する深度バッファを指定する
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	// 描画先キャンバスを塗りつぶす
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	// 深度バッファをリセットする
	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT strides = sizeof(VertexV);
	UINT offsets = 0;
	g_pDeviceContext->IASetInputLayout(g_pInputLayout);
	g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &strides, &offsets);
	g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	g_pDeviceContext->VSSetShader(g_pVertexShader, NULL, 0);
	g_pDeviceContext->PSSetShader(g_pPixelShader, NULL, 0);

	g_pDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF); // 混合ステートを設定（デフォルトのブレンドステートを使用）

	//BackGround: pTextureSRV2
	RenderImage(-1.f, -1.f, 4.75f, 4.75f, pTextureSRV2, 0, 1, 1);
	// 设置采样器状态

}

void RendererDrawB()
{
	// ダブルバッファの切り替えを行い画面を更新する
	g_pSwapChain->Present(0, 0);

}

void RendererUninit()
{
	// ※DirectXの各機能は作成した後、アプリ終了時に必ず解放しなければならない
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
	//拡張子csoのファイル名を作成
	char csoFileName[256];
	const char* dot = strrchr(szFileName, '.');  // 最後の '.' を探す
	if (dot) {
		int basenameLen = dot - szFileName;
		strncpy(csoFileName, szFileName, basenameLen); // 拡張子がある場合は拡張子を除いたファイル名をコピー
		csoFileName[basenameLen] = '\0';   // 終端文字を追加
	}
	else {
		strcpy(csoFileName, szFileName);   // 拡張子がない場合はそのままコピー
	}
	strcat(csoFileName, ".cso");// ".cso" 拡張子を付加

	//csoファイルがあれば開く
	FILE* fp;
	int ret = fopen_s(&fp, csoFileName, "rb");
	if (ret == 0)
	{
		// ファイルサイズを取得
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// バイナリデータを読み込み用にメモリ確保
		unsigned char* byteArray = new unsigned char[size];
		fread(byteArray, size, 1, fp);
		fclose(fp);

		// 呼び出し元にポインタとサイズを渡す
		*ppShaderObject = byteArray;
		*pShaderObjectSize = size;
	}
	//csoファイルがなければhlslファイルをコンパイルする
	else
	{
		HRESULT hr = S_OK;
		WCHAR	filename[512];
		size_t 	wLen = 0;
		int err = 0;

		// 文字コードを Shift-JIS → UTF-16 に変換
		setlocale(LC_ALL, "japanese");  // ロケールを設定（Windows特有）
		err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

		// シェーダーコンパイルオプションを設定
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG; // デバッグビルドの場合はデバッグ情報も含める
#endif

		// コンパイル結果およびエラー情報格納用のBlob
		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlob = nullptr;

		// HLSLファイルをコンパイル
		hr = D3DCompileFromFile(
			filename,							// ファイル名
			nullptr,							// マクロ定義なし 
			D3D_COMPILE_STANDARD_FILE_INCLUDE,	// #include 対応 
			szEntryPoint,						// エントリーポイント名
			szShaderModel,						// シェーダーモデル
			dwShaderFlags,						// コンパイルフラグ
			0,									// エフェクトフラグ
			&pBlob,								// 成功時のコンパイル結果
			&pErrorBlob);						// コンパイルエラー出力

		// コンパイル失敗時のエラーメッセージを表示
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr) {
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			if (pBlob)(pBlob)->Release();
			return E_FAIL;
		}

		// エラーブロブがあれば解放
		if (pErrorBlob) pErrorBlob->Release();

		// コンパイル成功時のバイナリデータを呼び出し元に渡す
		*ppShaderObject = (pBlob)->GetBufferPointer();
		*pShaderObjectSize = (pBlob)->GetBufferSize();
	}

	return S_OK;
}

HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ファイルの拡張子に合わせてコンパイル
	HRESULT hr = CompileShader(szFileName, "main", "vs_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return E_FAIL;

	// デバイスを使って頂点シェーダーを作成
	hr = g_pDevice->CreateVertexShader(ShaderObject, ShaderObjectSize, NULL, ppVertexShader);

	// デバイスを使って頂点レイアウトを作成
	g_pDevice->CreateInputLayout(pLayout, numElements, ShaderObject, ShaderObjectSize, ppVertexLayout);

	return S_OK;
}

HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ファイルの拡張子に合わせてコンパイル
	HRESULT hr = CompileShader(szFileName, "main", "ps_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーを生成
	hr = g_pDevice->CreatePixelShader(ShaderObject, ShaderObjectSize, nullptr, ppPixelShader);
	if (FAILED(hr)) return hr;

	return S_OK;
}

// 四边形绘制函数
void RenderQuad(const VertexV vertices[4], ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
	// 创建临时顶点缓冲区 [1,5](@ref)
	ID3D11Buffer* pQuadBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(VertexV) * 4;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices;
	g_pDevice->CreateBuffer(&bufferDesc, &initData, &pQuadBuffer);

	// 设置渲染管线状态 [2,5](@ref)
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	// 执行绘制命令 [1,3](@ref)
	g_pDeviceContext->Draw(4, 0);

	// 释放临时资源
	SAFE_RELEASE(pQuadBuffer);
}

void RenderImage(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV, int frameIndex = 0, int rows = 1, int columns = 1)
{
	// 计算精灵表中每个帧的尺寸（纹理坐标）
	float frameWidth = 1.0f / columns;
	float frameHeight = 1.0f / rows;

	// 计算当前帧在精灵表中的位置
	int row = frameIndex / columns;
	int col = frameIndex % columns;

	// 计算当前帧的纹理坐标范围
	float u0 = col * frameWidth;       // 左边界
	float u1 = (col + 1) * frameWidth; // 右边界
	float v0 = row * frameHeight;      // 上边界
	float v1 = (row + 1) * frameHeight;// 下边界

	// 创建顶点数据（包含正确的纹理坐标）
	VertexV vertices[4] = {
		// 位置坐标                   // 纹理坐标
		{ posX + width, posY + height, 0.5f, u1, v0 }, // 右上
		{ posX + width, posY,          0.5f, u1, v1 }, // 右下
		{ posX,         posY + height, 0.5f, u0, v0 }, // 左上
		{ posX,         posY,          0.5f, u0, v1 }  // 左下
	};

	// 创建临时顶点缓冲区
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
		// 错误处理
		return;
	}

	// 绑定顶点缓冲区
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &pDynamicBuffer, &stride, &offset);

	// 绑定纹理资源
	g_pDeviceContext->PSSetShaderResources(0, 1, &textureSRV);

	// 设置采样器状态
	g_pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

	g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF); // 设置混合状态

	// 绘制四边形
	g_pDeviceContext->Draw(4, 0);

	// 释放临时资源
	SAFE_RELEASE(pDynamicBuffer);
}

void RenderNumber(int number, float startX, float startY, float digitWidth, float digitHeight, ID3D11ShaderResourceView* textureSRV) {
	if (number < 0) return; // 仅支持非负数

	// 计算数字总位数（如123有3位）
	int numDigits = 0;
	int temp = number;
	while (temp > 0) {
		temp /= 10;
		numDigits++;
	}
	if (numDigits == 0) numDigits = 1; // 处理0的情况

	// 从右向左逐位渲染（对齐方式：右对齐）
	float currentX = startX + (numDigits - 1) * digitWidth; // 右对齐起点
	temp = number;
	for (int i = 0; i < numDigits; i++) {
		int digit = temp % 10; // 获取当前位（个位）
		temp /= 10;            // 移除已处理位

		// 渲染当前数字
		RenderImage(currentX, startY, digitWidth, digitHeight, textureSRV, digit, 1, 10);
		currentX -= digitWidth; // 左移一位
	}
}