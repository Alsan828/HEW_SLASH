#pragma once
#include <d3d11.h>
#include <string>
#include <unordered_map>

HRESULT LoadTexture(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** srv);



