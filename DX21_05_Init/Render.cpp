#include "Render.h"
#include "Game.h"
#include "Camera.h"
#include "Audio.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }


// グローバル変数を定義する（ここでは初期化可）
ID3D11Device* g_pDevice = nullptr;
ID3D11DeviceContext* g_pDeviceContext = nullptr;

ID3D11Buffer* g_pConstantBuffer = nullptr; // 11 月 12 日追加

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

static ConstantBuffer g_renderConstantData = {};

ID3D11ShaderResourceView* texIdle1 = nullptr;
ID3D11ShaderResourceView* texIdle2 = nullptr;
ID3D11ShaderResourceView* texIdle3 = nullptr;
ID3D11ShaderResourceView* texRun1 = nullptr;
ID3D11ShaderResourceView* texRun2 = nullptr;
ID3D11ShaderResourceView* texRun3 = nullptr;


D3D11_SAMPLER_DESC sampDesc = {};
ID3D11SamplerState* pSamplerState = nullptr;

static void UploadConstantBufferState()
{
	if (!g_pConstantBuffer || !g_pDeviceContext) {
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = g_pDeviceContext->Map(g_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		return;
	}

	memcpy(mappedResource.pData, &g_renderConstantData, sizeof(ConstantBuffer));
	g_pDeviceContext->Unmap(g_pConstantBuffer, 0);
	g_pDeviceContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}

static void ResetLinearClipState()
{
	g_renderConstantData.useLinearClip = 0.0f;
	g_renderConstantData.clipPlaneCount = 0.0f;

	for (int i = 0; i < MAX_LINEAR_CLIP_PLANES; ++i) {
		g_renderConstantData.clipPlanes[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 0.5f);
	}
}

HRESULT RendererInit(HWND hwnd) {
	HRESULT hr = S_OK;
	Audio::Init();

	// ゲーム内でよく使う SE だけ先に読み込む（プレイヤーや敵の SE など）
	Audio::PreloadSE(SoundEffect::ENEMY_HIT);
	Audio::PreloadSE(SoundEffect::ENEMY_DEATH);
	Audio::PreloadSE(SoundEffect::DASH);
	Audio::PreloadSE(SoundEffect::SHOOT);
	Audio::PreloadSE(SoundEffect::JUMP);

	// 実際のウィンドウクライアント領域サイズを取得する
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	UINT windowWidth = clientRect.right - clientRect.left;
	UINT windowHeight = clientRect.bottom - clientRect.top;

	// デバイスとスワップチェーンを作成する
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = windowWidth;      // 実際のウィンドウ幅を使う
	swapChainDesc.BufferDesc.Height = windowHeight;    // 実際のウィンドウ高を使う
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;  // ウィンドウモードのままにする（ボーダーレス全画面）

	// デバイスとスワップチェーンを同時に作成する
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

	// レンダーターゲットビューを作成する
	ID3D11Texture2D* renderTarget;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (FAILED(hr)) return hr;
	hr = g_pDevice->CreateRenderTargetView(renderTarget, NULL, &g_pRenderTargetView);
	renderTarget->Release();
	if (FAILED(hr)) return hr;

	// 深度ステンシルバッファを作成する
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = windowWidth;   // 実際のウィンドウ幅を使う
	textureDesc.Height = windowHeight; // 実際のウィンドウ高を使う
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

	// 深度ステンシルビューを作成する
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = 0;
	hr = g_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &g_pDepthStencilView);
	if (FAILED(hr)) return hr;
	depthStencile->Release();

	// ビューポートを作成する（実際のウィンドウサイズを使用）
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
		// 位置座標要素があることを示す
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// テクスチャ座標要素があることを示す
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 色成分用
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// 頂点シェーダーを作成し、同時に頂点レイアウトも作成する
	hr = CreateVertexShader(&g_pVertexShader, &g_pInputLayout, layout, numElements, "VertexShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return hr;
	}

	// ピクセルシェーダーを作成する
	hr = CreatePixelShader(&g_pPixelShader, "PixelShader.hlsl");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/back_img_01.png", &pTextureSRV2);
	if (FAILED(hr)) {
		// エラー処理
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	hr = LoadTexture(g_pDevice, "asset/UI/number.png", &pTextureNum);
	if (FAILED(hr)) {
		// エラー処理
		MessageBoxA(NULL, "OP error", "error", MB_OK);
		return hr;
	}

	// サンプラーステートを作成する
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	g_pDevice->CreateSamplerState(&sampDesc, &pSamplerState);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE; // Alpha to Coverage を無効化
	blendDesc.IndependentBlendEnable = FALSE; // 独立ブレンドを無効化
	blendDesc.RenderTarget[0].BlendEnable = TRUE; // レンダーターゲットのブレンドを有効化
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // ソースのブレンド係数
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // デスティネーションのブレンド係数
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // ブレンド演算
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha のソース係数
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha のデスティネーション係数
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha ブレンド演算
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 全カラー成分を書き込む
	hr = g_pDevice->CreateBlendState(&blendDesc, &g_pBlendState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBlendState error", "error", MB_OK);
		return hr;
	}

	ID3D11DepthStencilState* depthStencilState = nullptr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 深度書き込みマスク
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // 深度比較関数
	hr = g_pDevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateDepthStencilState error", "error", MB_OK);
		return hr;
	}
	g_pDeviceContext->OMSetDepthStencilState(depthStencilState, 1); // 深度ステンシルステートを設定する


	// 11 月 12 日追加
	// シェーダー用定数バッファ
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(ConstantBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = g_pDevice->CreateBuffer(&cbDesc, nullptr, &g_pConstantBuffer);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create constant buffer.", "Error", MB_OK);
		return hr;
	}

	g_renderConstantData.worldView = DirectX::XMMatrixIdentity();
	g_renderConstantData.projection = DirectX::XMMatrixIdentity();
	g_renderConstantData.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_renderConstantData.matrixTex = DirectX::XMMatrixIdentity();
	g_renderConstantData.fillRatio = 0.0f;
	g_renderConstantData.useGaugeFill = 0.0f;
	ResetLinearClipState();
	UploadConstantBufferState();
	


	return S_OK;
}


void RendererDrawF()
{
	// 画面クリア色
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // red, green, blue, alpha

	// 描画先のレンダーターゲットと深度バッファを指定する
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	// 描画先をクリアする
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

	g_pDeviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF); // ブレンドステートを設定する（既定を使用）

	// 背景: pTextureSRV2
	RenderImage(-1.f, -1.f, 4.75f, 4.75f, pTextureSRV2, 0, 1, 1);
	// サンプラーステート設定

}

void RendererDrawB()
{
    RenderOverlay();
	// ダブルバッファを切り替えて画面を更新する
	g_pSwapChain->Present(0, 0);

}

void RenderOverlay()
{
	// カスタムカーソルをすべての上に描画する。
	// ゲームプレイ中はカメラ追従、それ以外のシーンでは UI として扱う。
    if (g_gameState == STATE_PLAYING) {
        g_gameCursor.Render(g_camera.GetX(), g_camera.GetY());
    }
    else {
        g_gameCursor.Render(0.0f, 0.0f);
    }
}

void RendererUninit()
{
	Audio::Shutdown();
	// ※ 作成した DirectX リソースはアプリ終了時に必ず解放する
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
	SAFE_RELEASE(g_uiNumberTexture); // UI 数字用テクスチャ
	SAFE_RELEASE(pTextureNum);
	SAFE_RELEASE(g_pConstantBuffer); // 11 月 12 日追加
	SAFE_RELEASE(g_pBlendState);
	SAFE_RELEASE(pSamplerState);
}

HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ppShaderObject, int* pShaderObjectSize)
{
	// .cso ファイル名を作成する
	char csoFileName[256];
	const char* dot = strrchr(szFileName, '.');  // 最後の '.' を探す
	if (dot) {
		int basenameLen = (int)(dot - szFileName);
		strncpy(csoFileName, szFileName, basenameLen); // 拡張子がある場合は拡張子なしの名前をコピー
		csoFileName[basenameLen] = '\0';   // 終端文字を追加
	}
	else {
		strcpy(csoFileName, szFileName);   // 拡張子がなければそのままコピー
	}
	strcat(csoFileName, ".cso");// ".cso" 拡張子を付ける

	// .cso ファイルがあれば開く
	FILE* fp;
	int ret = fopen_s(&fp, csoFileName, "rb");
	if (ret == 0)
	{
		// ファイルサイズを取得する
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// バイナリ読み込み用メモリを確保する
		unsigned char* byteArray = new unsigned char[size];
		fread(byteArray, size, 1, fp);
		fclose(fp);

		// ポインタとサイズを呼び出し元へ返す
		*ppShaderObject = byteArray;
		*pShaderObjectSize = size;
	}
	// .cso が存在しない場合は .hlsl をコンパイルする
	else
	{
		HRESULT hr = S_OK;
		WCHAR	filename[512];
		size_t 	wLen = 0;
		int err = 0;

		// 文字コードを Shift-JIS から UTF-16 へ変換する
		setlocale(LC_ALL, "japanese");  // ロケール設定（Windows 固有）
		err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

		// シェーダーコンパイルオプションを設定する
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG; // デバッグビルド用にデバッグ情報を含める
#endif

		// コンパイル結果とエラー情報を保持する Blob
		ID3DBlob* pErrorBlob = nullptr;
		ID3DBlob* pBlob = nullptr;

		// HLSL ファイルをコンパイルする
		hr = D3DCompileFromFile(
			filename,							// ファイル名
			nullptr,							// マクロ定義なし
			D3D_COMPILE_STANDARD_FILE_INCLUDE,	// #include をサポート
			szEntryPoint,						// エントリーポイント名
			szShaderModel,						// シェーダーモデル
			dwShaderFlags,						// コンパイルフラグ
			0,									// エフェクトフラグ
			&pBlob,								// 成功時のコンパイル結果
			&pErrorBlob);						// コンパイルエラー出力

		// コンパイル失敗時はエラーメッセージを表示する
		if (FAILED(hr))
		{
			if (pErrorBlob != nullptr) {
				MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
			}
			if (pErrorBlob) pErrorBlob->Release();
			if (pBlob)(pBlob)->Release();
			return E_FAIL;
		}

		// エラー Blob があれば解放する
		if (pErrorBlob) pErrorBlob->Release();

		// 成功時はコンパイル済みバイナリを呼び出し元へ返す
		*ppShaderObject = (pBlob)->GetBufferPointer();
		*pShaderObjectSize = (int)(pBlob)->GetBufferSize();
	}

	return S_OK;
}

HRESULT CreateVertexShader(ID3D11VertexShader** ppVertexShader, ID3D11InputLayout** ppVertexLayout, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int numElements, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ファイル拡張子に応じてコンパイルする
	HRESULT hr = CompileShader(szFileName, "main", "vs_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return E_FAIL;

	// デバイスで頂点シェーダーを作成する
	hr = g_pDevice->CreateVertexShader(ShaderObject, ShaderObjectSize, NULL, ppVertexShader);

	// デバイスで頂点レイアウトを作成する
	g_pDevice->CreateInputLayout(pLayout, numElements, ShaderObject, ShaderObjectSize, ppVertexLayout);

	return S_OK;
}

HRESULT CreatePixelShader(ID3D11PixelShader** ppPixelShader, const char* szFileName)
{
	void* ShaderObject;
	int	ShaderObjectSize;

	// ファイル拡張子に応じてコンパイルする
	HRESULT hr = CompileShader(szFileName, "main", "ps_5_0", &ShaderObject, &ShaderObjectSize);
	if (FAILED(hr)) return hr;

	// ピクセルシェーダーを作成する
	hr = g_pDevice->CreatePixelShader(ShaderObject, ShaderObjectSize, nullptr, ppPixelShader);
	if (FAILED(hr)) return hr;

	return S_OK;
}

// 四角形描画関数
void RenderQuad(const VertexV vertices[4], ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
	// 一時的な頂点バッファを作成する
	ID3D11Buffer* pQuadBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(VertexV) * 4;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices;
	g_pDevice->CreateBuffer(&bufferDesc, &initData, &pQuadBuffer);

	// レンダリングパイプライン状態を設定する
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	// 描画コマンドを実行する
	g_pDeviceContext->Draw(4, 0);

	// 一時リソースを解放する
	SAFE_RELEASE(pQuadBuffer);
}

void RenderImage(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV,
	int frameIndex, int rows, int columns, bool enableCulling,
	float rotation, bool flipHorizontal)
{
	// 如果启用剔除且物体不可见，则跳过渲染
	if (enableCulling && !g_camera.IsRectVisible(posX, posY, width, height)) {
		return;
	}

	// スプライトシート内の 1 フレームサイズを計算する（テクスチャ座標）
	float frameWidth = 1.0f / columns;
	float frameHeight = 1.0f / rows;

	// スプライトシート内の現在フレーム位置を計算する
	// rows = 縦分割数、columns = 横分割数
	int col = frameIndex % columns;
	int row = frameIndex / columns;

	// 現在フレームのテクスチャ座標範囲を計算する
	float u0 = col * frameWidth;       // 左端
	float u1 = (col + 1) * frameWidth; // 右端
	float v0 = row * frameHeight;      // 上端
	float v1 = (row + 1) * frameHeight; // 下端

	// 应用水平翻转
	if (flipHorizontal) {
		float temp = u0;
		u0 = u1;
		u1 = temp;
	}

	// 回転用の矩形中心
	float centerX = posX + width * 0.5f;
	float centerY = posY + height * 0.5f;

	D3D11_SUBRESOURCE_DATA initData;
	// 回転がない場合は簡単な計算を使う
	if (rotation == 0.0f) {
		// 回転なしなら頂点位置を直接計算する
		VertexV vertices[4] = {
			{ posX + width, posY + height, 0.5f, u1, v0 }, // 右上
			{ posX + width, posY,           0.5f, u1, v1 }, // 右下
			{ posX,         posY + height, 0.5f, u0, v0 }, // 左上
			{ posX,         posY,          0.5f, u0, v1 }  // 左下
		};

		initData.pSysMem = vertices;
		// 頂点バッファを作成して使用する...
	}
	else {
		// 回転ありの場合は回転後の頂点位置を計算する
		float cosA = cosf(rotation);
		float sinA = sinf(rotation);

		auto rotatePoint = [&](float x, float y) {
			float dx = x - centerX;
			float dy = y - centerY;
			return std::pair<float, float>(
				centerX + dx * cosA - dy * sinA,
				centerY + dx * sinA + dy * cosA
			);
			};

		// 元の四隅（未回転）
		float x0 = posX;         float y0 = posY;          // 左下
		float x1 = posX + width; float y1 = posY;          // 右下
		float x2 = posX;         float y2 = posY + height; // 左上
		float x3 = posX + width; float y3 = posY + height; // 右上

		// 回転を適用する
		auto p0 = rotatePoint(x3, y3); // 右上
		auto p1 = rotatePoint(x1, y1); // 右下
		auto p2 = rotatePoint(x2, y2); // 左上
		auto p3 = rotatePoint(x0, y0); // 左下

		VertexV vertices[4] = {
			{ p0.first, p0.second, 0.5f, u1, v0 }, // 右上
			{ p1.first, p1.second, 0.5f, u1, v1 }, // 右下
			{ p2.first, p2.second, 0.5f, u0, v0 }, // 左上
			{ p3.first, p3.second, 0.5f, u0, v1 }  // 左下
		};
		initData.pSysMem = vertices;

		// 頂点バッファを作成して使用する...
	}

	// この後のバッファ生成・リソース設定・描画コードはそのまま...
	// 一時的な頂点バッファを作成する
	ID3D11Buffer* pDynamicBuffer = nullptr;
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(VertexV) * 4;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	HRESULT hr = g_pDevice->CreateBuffer(&desc, &initData, &pDynamicBuffer);
	if (FAILED(hr)) {
		// エラー処理
		return;
	}

	// 頂点バッファをバインドする
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &pDynamicBuffer, &stride, &offset);

	// テクスチャリソースをバインドする
	g_pDeviceContext->PSSetShaderResources(0, 1, &textureSRV);

	// サンプラーステートを設定する
	g_pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

	g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF); // ブレンドステートを設定する

	// 四角形を描画する
	g_pDeviceContext->Draw(4, 0);

	// 一時リソースを解放する
	SAFE_RELEASE(pDynamicBuffer);
}

// ゲーム内のゲージ描画用
void RenderGaugeFillImage(float posX, float posY, float width, float height,
	ID3D11ShaderResourceView* textureSRV, float fillRatio)
{
	if (textureSRV == nullptr || fillRatio <= 0.0f) return;

	fillRatio = min(max(fillRatio, 0.0f), 1.0f);

	g_renderConstantData.worldView = DirectX::XMMatrixIdentity();
	g_renderConstantData.projection = DirectX::XMMatrixIdentity();
	g_renderConstantData.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_renderConstantData.matrixTex = DirectX::XMMatrixIdentity();
	g_renderConstantData.fillRatio = fillRatio;
	g_renderConstantData.useGaugeFill = 1.0f;
	ResetLinearClipState();
	UploadConstantBufferState();

	// 通常描画する
	RenderImage(posX, posY, width, height, textureSRV, 0, 1, 1, false, 0.0f, false);

	// 次回描画のためにゲージ塗り設定を無効化する
	g_renderConstantData.fillRatio = 0.0f;
	g_renderConstantData.useGaugeFill = 0.0f;
	UploadConstantBufferState();
}

// 左へずれないように縦方向クリップで描画する
void RenderImageClipped(float posX, float posY, float width, float height, ID3D11ShaderResourceView* textureSRV, float fillRatio)
{
	// 左端を固定したまま右側から縮める
	float renderWidth = width * fillRatio;  // 実際に描画する幅

	// テクスチャ座標 - 描画幅に合わせて右側を切り取る
	float u0 = 0.0f;              // 左端
	float u1 = fillRatio;         // 右端 - 幅に合わせてトリミング
	float v0 = 0.0f;              // 上端
	float v1 = 1.0f;              // 下端

	D3D11_SUBRESOURCE_DATA initData;
	VertexV vertices[4] = {
		{ posX + renderWidth, posY + height, 0.5f, u1, v0 }, // 右上
		{ posX + renderWidth, posY,          0.5f, u1, v1 }, // 右下
		{ posX,               posY + height, 0.5f, u0, v0 }, // 左上
		{ posX,               posY,          0.5f, u0, v1 }  // 左下
	};

	initData.pSysMem = vertices;

	// 一時的な頂点バッファを作成する
	ID3D11Buffer* pDynamicBuffer = nullptr;
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(VertexV) * 4;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	HRESULT hr = g_pDevice->CreateBuffer(&desc, &initData, &pDynamicBuffer);
	if (FAILED(hr)) {
		return;
	}

	// 頂点バッファをバインドする
	UINT stride = sizeof(VertexV);
	UINT offset = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &pDynamicBuffer, &stride, &offset);

	// テクスチャリソースをバインドする
	g_pDeviceContext->PSSetShaderResources(0, 1, &textureSRV);

	// サンプラーステートを設定する
	g_pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);
	g_pDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xFFFFFFFF);

	// 四角形を描画する
	g_pDeviceContext->Draw(4, 0);

	// 一時リソースを解放する
	SAFE_RELEASE(pDynamicBuffer);
}

void RenderNumber(int number, float startX, float startY, float digitWidth, float digitHeight, ID3D11ShaderResourceView* textureSRV, bool enableCulling) {

	// 如果启用剔除且物体不可见，则跳过渲染
	if (enableCulling && !g_camera.IsRectVisible(startX, startY, digitWidth, digitHeight)) {
		return;
	}
	if (number < 0) return; // 非負の数のみ対応

	// 桁数を計算する（例: 123 は 3 桁）
	int numDigits = 0;
	int temp = number;
	while (temp > 0) {
		temp /= 10;
		numDigits++;
	}
	if (numDigits == 0) numDigits = 1; // 0 の場合を処理する

	// 右から左へ描画する（右揃え）
	float currentX = startX + (numDigits - 1) * digitWidth; // 右揃えの開始位置
	temp = number;
	for (int i = 0; i < numDigits; i++) {
		int digit = temp % 10; // 現在の桁を取得（一の位）
		temp /= 10;            // 処理済みの桁を削る

		// 現在の桁を描画する
		RenderImage(currentX, startY, digitWidth, digitHeight, textureSRV, digit, 1, 10);
		currentX -= digitWidth; // 1 桁分左へ移動
	}
}


	// 11 月 12 日追加
void SetColor(float r, float g, float b, float a)
{
	g_renderConstantData.color = DirectX::XMFLOAT4(r, g, b, a);
	g_renderConstantData.fillRatio = 0.0f;
	g_renderConstantData.useGaugeFill = 0.0f;
	ResetLinearClipState();
	UploadConstantBufferState();
}

void SetLinearClipPlanes(const LinearClipPlane* planes, int count)
{
	if (count < 0) {
		count = 0;
	}
	if (count > MAX_LINEAR_CLIP_PLANES) {
		count = MAX_LINEAR_CLIP_PLANES;
	}

	ResetLinearClipState();
	g_renderConstantData.useLinearClip = (count > 0) ? 1.0f : 0.0f;
	g_renderConstantData.clipPlaneCount = static_cast<float>(count);

	for (int i = 0; i < count; ++i) {
		const LinearClipPlane& plane = planes[i];
		float keepSide = (plane.keepSide >= 0.0f) ? 1.0f : -1.0f;
		g_renderConstantData.clipPlanes[i] = DirectX::XMFLOAT4(
			plane.normalX * keepSide,
			plane.normalY * keepSide,
			plane.centerU,
			plane.centerV
		);
	}

	UploadConstantBufferState();
}

void SetLinearClip(bool enabled, float normalX, float normalY, float centerU, float centerV, float keepSide)
{
	if (!enabled) {
		SetLinearClipPlanes(nullptr, 0);
		return;
	}

	LinearClipPlane plane = { normalX, normalY, centerU, centerV, keepSide };
	SetLinearClipPlanes(&plane, 1);
}
