#include "Texture1.h"
#include "stb_image.h"

HRESULT LoadTexture(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** srv)
{

    bool sts = true;
    unsigned char* pixels;

    int m_width; // テクスチャの幅
    int m_height; // テクスチャの高さ
    int m_bpp; // 1 ピクセルあたりのバイト数（BPP）

    // stb_image ライブラリで画像を読み込む
    pixels = stbi_load(filename, &m_width, &m_height, &m_bpp, 4);
    if (pixels == nullptr) {
        MessageBoxA(NULL, filename, "load error", MB_OK);
        return S_FALSE;
    }

    ID3D11Texture2D* pTexture;

    // テクスチャ記述子を設定する
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA 形式
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    // ピクセル情報を使ってサブリソースデータを設定する
    D3D11_SUBRESOURCE_DATA subResource{};
    subResource.pSysMem = pixels;
    subResource.SysMemPitch = desc.Width * 4; // RGBA = 1 ピクセル 4 バイト
    subResource.SysMemSlicePitch = 0;

    // DirectX 11 テクスチャを生成する
    HRESULT hr = device->CreateTexture2D(&desc, &subResource, &pTexture);
    if (FAILED(hr)) {
        stbi_image_free(pixels);
        return hr;
    }

    // テクスチャ用のシェーダーリソースビューを生成する
    hr = device->CreateShaderResourceView(pTexture, nullptr, srv);
    if (FAILED(hr)) {
        stbi_image_free(pixels);
        return hr;
    }

  /*  hr = device->CreateShaderResourceView(pTexture, nullptr, srv);
    if (SUCCEEDED(hr)) {
        OutputDebugStringA(("Loaded texture: " + std::string(filename) + "\n").c_str());
    }*/

    pTexture->Release();

    // 読み込んだピクセルデータを解放する（テクスチャはすでに GPU メモリ上にある）
    stbi_image_free(pixels);

    return S_OK;
}

// テクスチャを解放する
void ReleaseTexture(ID3D11ShaderResourceView*& texture)
{
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
}