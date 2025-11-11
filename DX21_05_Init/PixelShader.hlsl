Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 取出纹理颜色
    float4 texColor = tex.Sample(samLinear, input.texcoord);


    return texColor;
}