
struct VS_IN
{
    float4 pos : POSITION;
    float4 col : COLOR0; // for the color
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0; // for the color
    float2 texcoord : TEXCOORD0;
};
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = input.pos;
    output.texcoord = input.texcoord;
    
    output.col = input.col; // it forwards the color.   added november 12th

    return output;
}
