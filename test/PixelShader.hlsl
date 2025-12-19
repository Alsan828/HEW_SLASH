Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


// added november 12th
cbuffer ConstantBuffer : register(b0)
{
    matrix worldView;
    matrix projection;
    float4 color; // for the tint color
    matrix matrixTex;
};


struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0; // receive color from VS    added november 12th
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 取出纹理颜色
    float4 texColor = tex.Sample(samLinear, input.texcoord);

     // Multiply by vertex color (tinting)
    return texColor /** input.col */* color;  // added november 12th
    
    //return texColor;
}