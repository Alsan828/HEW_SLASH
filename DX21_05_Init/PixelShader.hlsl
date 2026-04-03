Texture2D tex : register(t0);
SamplerState samLinear : register(s0);

#define MAX_LINEAR_CLIP_PLANES 16

// added november 12th
cbuffer ConstantBuffer : register(b0)
{
    matrix worldView;  // 64 bytes
    matrix projection; // 64 bytes
    float4 color; // for the tint color // 16 bytes (because 4 floats)
    matrix matrixTex;  // 64 bytes
    
    float fillRatio; // used to check  how full is the gauge  // 4 bytes
    float useGaugeFill; // for checking is the gauge fill mode has to be used or not  // 4 bytes
    float useLinearClip; // arbitrary line clip toggle
    float clipPlaneCount; // how many clip planes are active
    float4 clipPlanes[MAX_LINEAR_CLIP_PLANES]; // normal.x, normal.y, centerU, centerV
    
    // ※ I need the padding because it has to be divided by 16 bc of the GPU, and 
    //    if I dont have the padding, it will not be correct 100% since it will be a decimal number.
};


struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0; // receive color from VS    added november 12th
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
    // 取出纹理颜色
    float4 texColor = tex.Sample(samLinear, input.texcoord);

    // Gauge fill logic (only runs when useGaugeFill is enabled)
    if (useGaugeFill > 0.5f)
    {
        // only apply the fill to non transparent pixels
        if (texColor.a > 0.01f)  // If pixel is not transparent
        {
            float fillThreshold = 1.0 - fillRatio;
            if (input.texcoord.y < fillThreshold)
            {
                discard; // it doesnt draw the pixel
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
    
    
     // Multiply by vertex color (tinting)
    return texColor /** input.col */* color;  // added november 12th
    
    //return texColor;
}

#undef APPLY_LINEAR_CLIP
