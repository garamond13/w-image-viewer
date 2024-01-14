#include "vs_out.hlsli"
#include "helpers.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    float theta; // x
    bool rotate; // y
}

float4 main(Vs_out vs_out) : SV_Target
{
    if (rotate)
        rotate_texcoord(vs_out.texcoord, theta);
    return tex.SampleLevel(smp, vs_out.texcoord, 0.0);
}
