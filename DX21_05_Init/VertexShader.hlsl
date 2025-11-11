
struct VS_IN
{
    float4 pos : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};
 
VS_OUT main(VS_IN input)
{
    VS_OUT output;
 
    output.pos = input.pos;
    output.texcoord = input.texcoord;

    return output;
}
