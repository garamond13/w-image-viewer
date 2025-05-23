#include "include\helpers.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    float theta;
    bool rotate;
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    if (rotate) {
        rotate_texcoord(texcoord, theta);
    }
    return tex.SampleLevel(smp, texcoord, 0.0);
}
