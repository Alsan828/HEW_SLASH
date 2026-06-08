Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

#define MAX_LINEAR_CLIP_PLANES 16

// 11 gatsu 12 nichi tsuika
cbuffer ConstantBuffer : register(b0)
{
    matrix worldView;  // 64 bytes
    matrix projection; // 64 bytes
    float4 color; // iromi henkou you no iro // 16 bytes (float ga 4 tsu)
    matrix matrixTex;  // 64 bytes
    
    float fillRatio; // ge-ji no juutenryou o kakunin suru tame ni tsukau // 4 bytes
    float useGaugeFill; // ge-ji juuten mo-do o tsukau ka douka o hantei suru // 4 bytes
    float useLinearClip; // nin'i no chokusen kurippu no kirikae
    float clipPlaneCount; // yuukou na kurippu heimensuu
    float4 clipPlanes[MAX_LINEAR_CLIP_PLANES]; // normal.x, normal.y, centerU, centerV
    
    // GPU no tsugou de 16 no baisuu ni soroeru hitsuyou ga aru tame padding ga hitsuyou.
    // padding ga nai to hasuu ni nari, tadashiku atsukaenai baai ga aru.
};


struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0; // VS kara iro o uketoru. 11 gatsu 12 nichi tsuika.
    float2 texcoord : TEXCOORD0;
};

#define APPLY_LINEAR_CLIP(index) \
    if (clipPlaneCount > (float)(index)) \
    { \
        float4 clipPlane = clipPlanes[index]; \
        float2 centered = input.texcoord - clipPlane.zw; \
        float side = dot(centered, clipPlane.xy); \
        if (side < 0.0f) \
        { \
            discard; \
        } \
    }

float4 main(PS_INPUT input) : SV_TARGET
{
    // tekusucha iro o shutoku suru
    float4 texColor = tex.Sample(samLinear, input.texcoord);

    // ge-ji juuten rojikku (useGaugeFill ga yuukou na toki dake jikkou)
    if (useGaugeFill > 0.5f)
    {
        // toumei de nai pikuseru ni dake juuten o tekiyou suru
        if (texColor.a > 0.01f)  // pikuseru ga toumei de nai baai
        {
            float fillThreshold = 1.0 - fillRatio;
            if (input.texcoord.y < fillThreshold)
            {
                discard; // kono pikuseru wa byouga shinai
            }
        }
    }

    if (useLinearClip > 0.5f)
    {
        APPLY_LINEAR_CLIP(0);
        APPLY_LINEAR_CLIP(1);
        APPLY_LINEAR_CLIP(2);
        APPLY_LINEAR_CLIP(3);
        APPLY_LINEAR_CLIP(4);
        APPLY_LINEAR_CLIP(5);
        APPLY_LINEAR_CLIP(6);
        APPLY_LINEAR_CLIP(7);
        APPLY_LINEAR_CLIP(8);
        APPLY_LINEAR_CLIP(9);
        APPLY_LINEAR_CLIP(10);
        APPLY_LINEAR_CLIP(11);
        APPLY_LINEAR_CLIP(12);
        APPLY_LINEAR_CLIP(13);
        APPLY_LINEAR_CLIP(14);
        APPLY_LINEAR_CLIP(15);
    }
    
    
     // chouten iro o kakeawase te iromi o kaeru
    return texColor /** input.col */* color;  // 11 gatsu 12 nichi tsuika
    
    //return texColor;
}

#undef APPLY_LINEAR_CLIP

