#pragma once
#include <d3d11.h>

HRESULT LoadTexture(ID3D11Device* device, const char* filename, ID3D11ShaderResourceView** srv);

#pragma once
