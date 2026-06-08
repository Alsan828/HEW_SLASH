
struct VS_IN
{
    float4 pos : POSITION;
    float4 col : COLOR0; // iro jouhou you
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0; // iro jouhou you
    float2 texcoord : TEXCOORD0;
};
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = input.pos;
    output.texcoord = input.texcoord;
    
    output.col = input.col; // iro jouhou o sono mama watasu. 11 gatsu 12 nichi tsuika.

    return output;
}

